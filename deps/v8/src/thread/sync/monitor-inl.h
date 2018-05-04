#ifndef THREAD_MONITOR_INL_H_
#define THREAD_MONITOR_INL_H_

#include <sstream>
#include "src/thread/sync/monitor.h"
#include "src/thread/sync/lock-word-inl.h"
#include "src/objects-inl.h"
#include "src/assert-scope.h"
#include "src/thread/thread-state-change-inl.h"

namespace v8 {
namespace internal {
namespace sync {

template<class Object>
bool RWMonitor::IsLocked(Handle<Object> obj, RWMonitor::LockType lock_type) {
  AtomicInteger* state =
      reinterpret_cast<AtomicInteger*>(obj->monitor_address());
  uint32_t value = Decode(state->LoadRelaxed());
  if (lock_type == RLOCK) {
    return value > kDefaultUnlocked;
  } else if (lock_type == WLOCK) {
    return value < kDefaultUnlocked;
  } else if (lock_type == RWLOCK) {
    return value != kDefaultUnlocked;
  } else {
    return value == kDefaultUnlocked;
  }
}


template<class Object>
bool RWMonitor::Lock(VMThread* self,
                     Handle<Object> obj,
                     RWMonitor::LockType type,
                     RWMonitor::StateMode mode) {
#ifdef DEBUG
  int obj_type = obj->map()->instance_type();
#endif
  if (type == RLOCK) {
    bool done = false;
    do {
      DCHECK(obj_type == obj->map()->instance_type());
      AtomicInteger* state =
          reinterpret_cast<AtomicInteger*>(obj->monitor_address());
      int32_t old_value = state->LoadRelaxed();
      int32_t cur_state = Decode(old_value);
      if (V8_LIKELY(cur_state >= kDefaultUnlocked)) {
        int32_t new_value = Encode(cur_state + 1);
        done = state->CompareExchangeWeakAcquire(old_value, new_value);
      } else {
        VMThreadState new_state = (mode == TRANSITION) ?
            kWait: self->state();
        ThreadStateChangeScope tsc(self, new_state);

        struct timespec timeout;
        timeout.tv_sec = FLAG_rwmonitor_timeout;
        timeout.tv_nsec = 0;
        if (futex(state->Address(), FUTEX_WAIT,
                  old_value, &timeout, NULL, 0) != 0 &&
            errno != EAGAIN) {
            V8_Fatal(__FILE__, __LINE__,
                     "futex wait (monitor=%d, mode=%d) timeout %lds for %s",
                     cur_state, mode, timeout.tv_sec, "RWMonitor::RLock");
        }
      }
    } while (!done);
  } else if (type == WLOCK) {
    bool done = false;
    do {
      DCHECK(obj_type == obj->map()->instance_type());
      AtomicInteger* state =
          reinterpret_cast<AtomicInteger*>(obj->monitor_address());
      int32_t old_value = state->LoadRelaxed();
      int32_t cur_state = Decode(old_value);
      if (V8_LIKELY(cur_state == kDefaultUnlocked)) {
        int32_t new_value = Encode(cur_state - 1);
        done = state->CompareExchangeWeakAcquire(old_value, new_value);
      } else {
        VMThreadState new_state = (mode == TRANSITION) ?
            kWait: self->state();
        ThreadStateChangeScope tsc(self, new_state);

        struct timespec timeout;
        timeout.tv_sec = FLAG_rwmonitor_timeout;
        timeout.tv_nsec = 0;
        if (futex(state->Address(), FUTEX_WAIT, old_value,
                  &timeout, NULL, 0) != 0 &&
            errno != EAGAIN) {
            V8_Fatal(__FILE__, __LINE__,
                     "futex wait (monitor=%d, mode=%d) timeout %lds for %s",
                     cur_state, mode, timeout.tv_sec, "RWMonitor::WLock");
        }
      }
    } while (!done);
  } else {
    DCHECK(type == NO_LOCK);
  }
  return true;
}


template<class Object>
void RWMonitor::Unlock(Handle<Object> obj, RWMonitor::LockType type) {
  if (type == RLOCK) {
    bool done = false;
    do {
      AtomicInteger* state =
          reinterpret_cast<AtomicInteger*>(obj->monitor_address());
      int32_t old_value = state->LoadRelaxed();
      int32_t cur_state = Decode(old_value);
      if (V8_LIKELY(cur_state > kDefaultUnlocked)) {
        int32_t new_value = Encode(cur_state - 1);
        done = state->CompareExchangeWeakAcquire(old_value, new_value);
        if (done) futex(state->Address(), FUTEX_WAKE, -1, NULL, NULL, 0);
      } else {
        V8_Fatal(__FILE__, __LINE__,
                 "Unexpected state %d for ReaderUnLock", cur_state);
      }
    } while (!done);
  } else if (type == WLOCK) {
    bool done = false;
    do {
      AtomicInteger* state =
          reinterpret_cast<AtomicInteger*>(obj->monitor_address());
      int32_t old_value = state->LoadRelaxed();
      int32_t cur_state = Decode(old_value);
      if (V8_LIKELY(cur_state < kDefaultUnlocked)) {
        int32_t new_value = Encode(cur_state + 1);
        done = state->CompareExchangeWeakSequentiallyConsistent(
                   old_value, new_value);
        if (done) futex(state->Address(), FUTEX_WAKE, -1, NULL, NULL, 0);
      } else {
        V8_Fatal(__FILE__, __LINE__,
                 "Unexpected state %d for WriterUnLock", cur_state);
      }
    } while(!done);
  } else {
    DCHECK(NO_LOCK);
  }
}


template<class Object>
void Monitor::LockInternal(VMThread* self, Handle<Object> obj) {
  monitor_lock_.Lock(self);
  while(true) {
    if (owner_ == NULL) {  // Unowned.
      owner_ = self;
      CHECK_EQ(lock_count_, 0);
      monitor_lock_.Unlock(self);
      CompeteLockWithIC(self, obj);
      return;
    } else if (owner_ == self) {
      lock_count_++;  // Recursive.
      monitor_lock_.Unlock(self);
      return;
    }
    ++num_waiters_;
    monitor_lock_.Unlock(self);  // Let go of locks in order.
    {
      ThreadStateChangeScope tsc(self, kWait);
      MutexLock mu2(self, monitor_lock_);
      if (owner_ != NULL) {  // Did the owner_ give the lock up?
        monitor_contenders_.Wait(self);  // Still contended so wait.
      }
    }
    monitor_lock_.Lock(self);  // Reacquire locks in order.
    --num_waiters_;
  }
}


template<class Object>
void Monitor::CompeteLockWithIC(VMThread* self, Handle<Object> obj) {
  LockWord new_value(this, LockWord::kStateFatLocked);
  uint32_t* monitor_address = NULL;

  while (true) {
    monitor_address = reinterpret_cast<uint32_t*>(obj->monitor_address());
    LockWord lw(*monitor_address);
    if (lw.GetState() == LockWord::kFatMode) {
      if (CasLockWordWeakSequentiallyConsistent(
            reinterpret_cast<uint8_t*>(monitor_address),
            lw.GetValue(),
            new_value.GetValue())) {
        break;
      }
    }
    self->CheckSuspend();
    sched_yield();
  }
}


template<class Object>
bool Monitor::UnlockInternal(VMThread* self, Handle<Object> obj) {
  DCHECK(self != NULL);
  MutexLock mu(self, monitor_lock_);
  VMThread* owner = owner_;
  if (owner == self) {
    // We own the monitor, so nobody else can be in here.
    if (lock_count_ == 0) {
      LockWord new_value(this);
      obj->set_monitor(new_value.GetValue());
      owner_ = NULL;
      // Wake a contender.
      monitor_contenders_.Signal(self);
    } else {
      --lock_count_;
    }
  } else {
    // We don't own this, so we're not allowed to unlock it.
    // Failure.
    Isolate* isolate = self->isolate();
    isolate->Throw(*isolate->factory()->
        NewError(MessageTemplate::kIllegalMonitorUnlock));
    return false;
  }
  return true;
}


template<class Object>
void Monitor::Lock(VMThread* self, Handle<Object> obj) {
  DCHECK(AllowHeapAllocation::IsAllowed() || self->IsInSuspendAll());
  uint32_t thread_id = self->thread_id();
  size_t contention_count = 0;

  while (true) {
    LockWord lock_word(obj->monitor());
    switch (lock_word.GetState()) {
      case LockWord::kUnlocked: {
        LockWord thin_locked(LockWord::FromThinLockId(thread_id, 0));
        // JSMT: TODO no_gc
        uint8_t* raw_addr = obj->monitor_address();
        if (CasLockWordWeakSequentiallyConsistent(raw_addr,
                                                  lock_word.GetValue(),
                                                  thin_locked.GetValue())) {
          return;  // Success!
        }
        continue;  // Go again.
      }
      case LockWord::kThinLocked: {
        if (lock_word.IsLockedByIC()) {
          ThreadStateChangeScope tcs(self, kWait);
          sched_yield();
          continue;
        }
        uint32_t owner_thread_id = lock_word.ThinLockOwner();
        if (owner_thread_id == thread_id) {
          uint32_t new_count = lock_word.ThinLockCount() + 1;
          if (V8_LIKELY(new_count <= LockWord::kThinLockMaxCount)) {
            LockWord thin_locked(LockWord::FromThinLockId(thread_id,
                                                          new_count));
            /*ObjectLockCheck's flag should not be clean*/
            if (lock_word.GetObjectLockCheck()) {
              thin_locked = LockWord::FromThinLockIdChecked(thread_id, new_count);
            }
            obj->set_monitor(thin_locked.GetValue());
            return;
          } else {
            InflateThinLocked(self, obj, lock_word);
          }
        } else {
          contention_count++;
          if (contention_count <= kDefaultMaxSpinsBeforeThinLockInflation) {
#ifdef _WIN32
            ::Sleep(0);
#else
            sched_yield();
#endif
          } else {
            // (JSMT): Because store-ic won't wake up any waiting threads.
            DCHECK(!lock_word.IsLockedByIC());
            contention_count = 0;
            InflateThinLocked(self, obj, lock_word);
          }
        }
        continue;  // Start from the beginning.
      }
      case LockWord::kFatMode:
      case LockWord::kFatLocked: {
        Monitor* mon = lock_word.FatLockMonitor(self);
        mon->LockInternal<Object>(self, obj);
        return;  // Success!
      }
      default: {
        V8_Fatal(__FILE__, __LINE__, "Invalid monitor state %d",
                 lock_word.GetState());
      }
    }
  }
}


template<class Object>
bool Monitor::Unlock(VMThread* self, Handle<Object> obj) {
  LockWord lock_word(obj->monitor());
  switch (lock_word.GetState()) {
    case LockWord::kFatMode:
    case LockWord::kUnlocked: {
      // Failure.
      Isolate* isolate = self->isolate();
      isolate->Throw(*isolate->factory()->NewError(
            MessageTemplate::kIllegalMonitorUnlockUnOwned));
      return false;
    }
    case LockWord::kThinLocked: {
      uint32_t thread_id = self->thread_id();
      uint32_t owner_thread_id = lock_word.ThinLockOwner();
      if (owner_thread_id != thread_id) {
        // Failure.
        Isolate* isolate = self->isolate();
        isolate->Throw(*isolate->factory()
            ->NewError(MessageTemplate::kIllegalMonitorUnlock));
        return false;
      } else {
        LockWord new_lw = LockWord::Default();
        if (lock_word.ThinLockCount() != 0) {
          uint32_t new_count = lock_word.ThinLockCount() - 1;
          new_lw = LockWord::FromThinLockId(thread_id, new_count);
          if (lock_word.GetObjectLockCheck()) {
            new_lw = LockWord::FromThinLockIdChecked(thread_id, new_count);
          }
        }
        obj->set_monitor(new_lw.GetValue());
        return true;  // Success!
      }
    }
    case LockWord::kFatLocked: {
      Monitor* mon = lock_word.FatLockMonitor(self);
      return mon->UnlockInternal<Object>(self, obj);
    }
    default: {
      V8_Fatal(__FILE__, __LINE__, "Invalid monitor state %d",
               lock_word.GetState());
    }
  }
  return true;
}

template<class Object>
void Monitor::InflateThinLocked(
    VMThread* self, Handle<Object> obj, LockWord lock_word) {
  uint32_t owner_thread_id = lock_word.ThinLockOwner();
  if (owner_thread_id == self->thread_id()) {
    // We own the monitor, we can easily inflate it.
    Inflate(self, self, obj);
  } else {
    bool timed_out;
    VMThreadList* thread_list = self->thread_list();

    VMThread* owner;
    {
      ThreadSuspensionScope tss(self);
      owner = thread_list->SuspendThreadByThreadId(owner_thread_id, &timed_out);
    }
    if (owner != NULL) {
      // We succeeded in suspending the thread, check the
      // lock's status didn't change.
      lock_word = LockWord(obj->monitor());
      if (lock_word.GetState() == LockWord::kThinLocked &&
          lock_word.ThinLockOwner() == owner_thread_id) {
        // Go ahead and inflate the lock.
        Inflate(self, owner, obj);
      }
      thread_list->Resume(owner);
    }
  }
}


template<class Object>
void Monitor::Inflate(VMThread* self, VMThread* owner, Handle<Object> obj) {
  Monitor* m = MonitorPool::CreateMonitor(self, owner);
  DCHECK(m != NULL);
  if (m->Install(self, obj)) {
    LockWord lw(obj->monitor());
    CHECK_EQ(lw.GetState(), LockWord::kFatLocked);
    self->thread_list()->monitor_list()->Add(m);
  } else {
#ifdef DEBUG
    m->Clear();
#endif
    MonitorPool::ReleaseMonitor(self, m);
  }
}


void Monitor::Clear() {
  num_waiters_ = 0;
  owner_ = NULL;
  wait_set_ = NULL;
  lock_count_ = 0;
  is_marked_ = false;
}


template<class Object>
bool Monitor::Install(VMThread* self, Handle<Object> obj) {
  MutexLock mu(self, monitor_lock_);
  LockWord lw(obj->monitor());
  switch (lw.GetState()) {
    case LockWord::kThinLocked: {
      CHECK_EQ(owner_->thread_id(), lw.ThinLockOwner());
      lock_count_ = lw.ThinLockCount();
      break;
    }
    case LockWord::kFatMode:
    case LockWord::kFatLocked: {
      return false;
    }
    case LockWord::kUnlocked: {
      V8_Fatal(__FILE__, __LINE__, "Inflating unlocked lock word");
      break;
    }
    default: {
      V8_Fatal(__FILE__, __LINE__, "Invalid monitor state %d",
               lw.GetState());
      return false;
    }
  }

  // JSMT: TODO no_gc
  LockWord fat(this, LockWord::kStateFatLocked);
  uint8_t* raw_addr = obj->monitor_address();
  return CasLockWordWeakSequentiallyConsistent(raw_addr,
                                               lw.GetValue(),
                                               fat.GetValue());
}

template<class Object>
void Monitor::Notify(VMThread* self, Handle<Object> obj, bool notify_all) {
  DCHECK(self != NULL);
  LockWord lock_word(obj->monitor());
  switch (lock_word.GetState()) {
    case LockWord::kUnlocked:
    case LockWord::kThinLocked: {
      return;  // Success.
    }
    case LockWord::kFatMode:
    case LockWord::kFatLocked: {
      Monitor* mon = lock_word.FatLockMonitor(self);
      if (notify_all) {
        mon->NotifyAllInternal(self);
      } else {
        mon->NotifyInternal(self);
      }
      return;  // Success.
    }
    default: {
      V8_Fatal(__FILE__, __LINE__, "Invalid monitor state %d",
               lock_word.GetState());
      return;
    }
  }
}


template<class Object>
bool Monitor::Wait(VMThread* self,
                   Handle<Object> obj,
                   const base::TimeDelta& rel_time) {
  DCHECK(self != NULL);
  LockWord lock_word(obj->monitor());
  while (lock_word.GetState() != LockWord::kFatLocked) {
    switch (lock_word.GetState()) {
      case LockWord::kThinLocked: {
        uint32_t thread_id = self->thread_id();
        uint32_t owner_thread_id = lock_word.ThinLockOwner();
        if (thread_id != owner_thread_id) {
          Isolate* isolate = self->isolate();
          isolate->Throw(*isolate->factory()
              ->NewError(MessageTemplate::kIllegalMonitorWait));
          return false;  // Failure.
        } else {
          Inflate(self, self, obj);
          lock_word = LockWord(obj->monitor());
        }
        break;
      }
      case LockWord::kFatMode:
      case LockWord::kUnlocked: {
        Isolate* isolate = self->isolate();
        isolate->Throw(*isolate->factory()
            ->NewError(MessageTemplate::kIllegalMonitorWait));
        return false;  // Failure.
      }
      case LockWord::kFatLocked:
      default: {
        V8_Fatal(__FILE__, __LINE__, "Invalid monitor state %d",
                 lock_word.GetState());
      }
    }
  }
  Monitor* mon = lock_word.FatLockMonitor(self);
  return mon->WaitInternal<Object>(self, obj, rel_time);
}


template<class Object>
bool Monitor::WaitInternal(VMThread* self, Handle<Object> obj,
                           const base::TimeDelta& rel_time) {
  DCHECK(self != NULL);
  monitor_lock_.Lock(self);
  // Make sure that we hold the lock.
  if (owner_ != self) {
    monitor_lock_.Unlock(self);
    Isolate* isolate = self->isolate();
    isolate->Throw(*isolate->factory()
        ->NewError(MessageTemplate::kIllegalMonitorWait));
    return false;
  }

  AppendToWaitSet(self);
  ++num_waiters_;
  int prev_lock_count = lock_count_;
  lock_count_ = 0;
  owner_ = NULL;
  obj->set_monitor(LockWord(this).GetValue());

  {
    ThreadStateChangeScope tsc(self, kWait);
    MutexLock mu(self, *self->wait_mutex());
    DCHECK(self->wait_monitor() == NULL);
    self->set_wait_monitor(this);

    // Release the monitor lock.
    monitor_contenders_.Signal(self);
    monitor_lock_.Unlock(self);
    if (rel_time.InMicroseconds() == 0) {
      self->wait_condition_variable()->Wait(self);
    } else {
      self->wait_condition_variable()->TimedWait(self, rel_time);
    }
  }

  {
    MutexLock mu(self, *self->wait_mutex());
    DCHECK(self->wait_monitor() != NULL);
    self->set_wait_monitor(NULL);
  }

  // Re-acquire the monitor and lock.
  LockInternal<Object>(self, obj);
  monitor_lock_.Lock(self);
  self->wait_mutex()->AssertNotHeld(self);

  owner_ = self;
  lock_count_ = prev_lock_count;
  --num_waiters_;
  RemoveFromWaitSet(self);
  monitor_lock_.Unlock(self);
  return true;
}


template<class Object>
void Monitor::Deflate(VMThread* self, Object* obj, bool is_locked) {
  // JSMT: TODO should lock ?
  // MutexLock mu(self, monitor_lock_);
  if (!FLAG_deflate_monitors || (num_waiters_ > 0)) {
    is_marked_ = true;
    return;
  }
  if (!is_locked) {
    DCHECK(owner_ == NULL);
    // No lock just put an empty lock word inside the object.
    LockWord new_lw = LockWord::Default();
    // Deflated obj to empty lock word
    obj->set_monitor(new_lw.GetValue());
    return;
  }

  if (owner_ != NULL) {
    // Can't deflate if our lock count is too high.
    if (lock_count_ > LockWord::kThinLockMaxCount) {
      is_marked_ = true;
      return;
    }
    // Deflate to a thin lock.
    LockWord new_lw = LockWord::FromThinLockId(owner_->thread_id(),
                                             lock_count_);
    obj->set_monitor(new_lw.GetValue());
  } else {
    DCHECK(lock_count_ == 0);
    // Deflate ic lock.
    obj->set_monitor(LockWord::ICLockWord());
  }
  return;
}


template<class Object>
void Monitor::Visit(VMThread* self, Object* obj) {
  LockWord lw(obj->monitor());
  // if obj has monitor, set is_marked_ true.
  if (lw.GetState() == LockWord::kFatMode ||
      lw.GetState() == LockWord::kFatLocked) {
    Monitor* m = lw.FatLockMonitor(self);
    bool is_locked = (lw.GetState() == LockWord::kFatLocked);
    m->Deflate<Object>(self, obj, is_locked);
  }
}

}  // namespace sync
}  // namespace internal
}  // namespace v8

#endif  // THREAD_MONITOR_INL_H_
