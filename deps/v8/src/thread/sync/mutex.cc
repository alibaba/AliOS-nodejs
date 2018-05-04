
// The source is based on Android Runtime VM.
// https://source.android.com/source/downloading.html


#include <errno.h>
#include <sys/time.h>
#include <set>
#include <atomic>
#include "src/thread/sync/mutex-inl.h"
#include "src/thread/thread-state-change-inl.h"
#include "src/assert-scope.h"
#include "src/thread/vmthread-inl.h"

namespace v8 {
namespace internal {
namespace sync {

#if 0
struct AllMutexData {
  // A guard for all_mutexes_ that's not a mutex
  // (Mutexes must CAS to acquire and busy wait).
  AtomicValue<BaseMutex*> all_mutexes_guard;
  // All created mutexes guarded by all_mutexes_guard_.
  std::set<BaseMutex*>* all_mutexes;
  AllMutexData() : all_mutexes(NULL) {}
};
static struct AllMutexData gAllMutexData[kAllMutexDataSize];


class ScopedAllMutexesLock final {
 public:
  explicit ScopedAllMutexesLock(BaseMutex* mutex) : mutex_(mutex) {
    while (!gAllMutexData->all_mutexes_guard.CompareExchangeWeakAcquire(
        0, mutex)) {
      base::OS::Sleep(base::TimeDelta::FromNanoseconds(100));
    }
  }

  ~ScopedAllMutexesLock() {
#if !defined(__clang__)
    // TODO: remove this workaround target GCC/libc++/bionic bug
    // "invalid failure memory model".
    while (!gAllMutexData->
        all_mutexes_guard.CompareExchangeWeakSequentiallyConsistent(
            mutex_, 0)) {
#else
    while (!gAllMutexData->
        all_mutexes_guard.CompareExchangeWeakRelease(mutex_, 0)) {
#endif
      base::OS::Sleep(base::TimeDelta::FromNanoseconds(100));
    }
  }

 private:
  BaseMutex* mutex_;
};
#endif


// Scoped class that generates events at the beginning
// and end of lock contention.
class ScopedContentionRecorder final {
 public:
  ScopedContentionRecorder(BaseMutex* mutex,
                           uint64_t blocked_tid,
                           uint64_t owner_tid)
      : mutex_(kLogLockContentions ? mutex : NULL),
        blocked_tid_(kLogLockContentions ? blocked_tid : 0),
        owner_tid_(kLogLockContentions ? owner_tid : 0),
        start_nano_time_(kLogLockContentions ?
                         base::Time::Now().ToInternalValue() : 0) {
  }

  ~ScopedContentionRecorder() {
    // ATRACE_END();
    if (kLogLockContentions) {
      uint64_t end_nano_time = base::Time::Now().ToInternalValue();
      mutex_->RecordContention(blocked_tid_,
                               owner_tid_,
                               end_nano_time - start_nano_time_);
    }
  }

 private:
  BaseMutex* mutex_;
  const uint64_t blocked_tid_;
  const uint64_t owner_tid_;
  const uint64_t start_nano_time_;
};


BaseMutex::BaseMutex(const char* name, LockLevel level)
  : level_(level), name_(name) {
#if 0
  if (kLogLockContentions) {
    // ScopedAllMutexesLock mu(this);
    std::set<BaseMutex*>** all_mutexes_ptr = &gAllMutexData->all_mutexes;
    if (*all_mutexes_ptr == NULL) {
      // We leak the global set of all mutexes to avoid ordering issues
      // in global variable construction/destruction.
      *all_mutexes_ptr = new std::set<BaseMutex*>();
    }
    (*all_mutexes_ptr)->insert(this);
  }
#endif
}


BaseMutex::~BaseMutex() {
#if 0
  if (kLogLockContentions) {
    ScopedAllMutexesLock mu(this);
    gAllMutexData->all_mutexes->erase(this);
  }
#endif
}


void BaseMutex::DumpAll(std::ostream& os) {
#if 0
  if (kLogLockContentions) {
    os << "Mutex logging:\n";
    ScopedAllMutexesLock mu(reinterpret_cast<BaseMutex*>(-1));
    std::set<BaseMutex*>* all_mutexes = gAllMutexData->all_mutexes;
    if (all_mutexes == NULL) {
      // No mutexes have been created yet during at startup.
      return;
    }
    typedef std::set<BaseMutex*>::const_iterator It;
    os << "(Contended)\n";
    for (It it = all_mutexes->begin(); it != all_mutexes->end(); ++it) {
      BaseMutex* mutex = *it;
      if (mutex->HasEverContended()) {
        mutex->Dump(os);
        os << "\n";
      }
    }
    os << "(Never contented)\n";
    for (It it = all_mutexes->begin(); it != all_mutexes->end(); ++it) {
      BaseMutex* mutex = *it;
      if (!mutex->HasEverContended()) {
        mutex->Dump(os);
        os << "\n";
      }
    }
  }
#endif
}


void BaseMutex::CheckSafeToWait(VMThread* self) {
  if (self == NULL) {
    CheckUnattachedVMThread(level_);
    return;
  }
  if (kDebugLocking) {
    CHECK(self->held_mutex(level_) == this || level_ == kDefaultMutexLevel);
    for (int i = kLockLevelCount - 1; i >= 0; --i) {
      if (i != level_) {
        BaseMutex* held_mutex = self->held_mutex(static_cast<LockLevel>(i));
        // We expect waits to happen while holding the
        // thread list suspend thread lock.
        CHECK(held_mutex == NULL);
      }
    }
  }
}


void BaseMutex::ContentionLogData::AddToWaitTime(size_t value) {
  if (kLogLockContentions) {
    // Atomically add value to wait_time.
    wait_time.FetchAndAddSequentiallyConsistent(value);
  }
}


void BaseMutex::RecordContention(uint64_t blocked_tid,
                                 uint64_t owner_tid,
                                 uint64_t nano_time_blocked) {
#if DEBUG
  ContentionLogData* data = contention_log_data_;
  ++(data->contention_count);
  data->AddToWaitTime(nano_time_blocked);
  ContentionLogEntry* log = data->contention_log;
  // This code is intentionally racy as it is only used for diagnostics.
  uint32_t slot = data->cur_content_log_entry.LoadRelaxed();
  if (log[slot].blocked_tid == blocked_tid &&
      log[slot].owner_tid == blocked_tid) {
    ++log[slot].count;
  } else {
    uint32_t new_slot;
    do {
      slot = data->cur_content_log_entry.LoadRelaxed();
      new_slot = (slot + 1) % kContentionLogSize;
    } while (!data->cur_content_log_entry.CompareExchangeWeakRelaxed(slot,
                                                                     new_slot));
    log[new_slot].blocked_tid = blocked_tid;
    log[new_slot].owner_tid = owner_tid;
    log[new_slot].count.StoreRelaxed(1);
  }
#endif
}


void BaseMutex::DumpContention(std::ostream& os) {
#if 0
#if DEBUG
  ContentionLogData* data = contention_log_data_;
  ContentionLogEntry* log = data->contention_log;
  uint64_t wait_time = data->wait_time.LoadRelaxed();
  uint32_t contention_count = data->contention_count.LoadRelaxed();
  if (contention_count == 0) {
    os << "never contended";
  } else {
    os << "contended " << contention_count
      << " total wait of contender " << wait_time
      << " average " << (wait_time / contention_count);
    std::map<uint64_t, size_t> most_common_blocker;
    std::map<uint64_t, size_t> most_common_blocked;
    for (size_t i = 0; i < kContentionLogSize; ++i) {
      uint64_t blocked_tid = log[i].blocked_tid;
      uint64_t owner_tid = log[i].owner_tid;
      uint32_t count = log[i].count.LoadRelaxed();
      if (count > 0) {
        auto it = most_common_blocked.find(blocked_tid);
        if (it != most_common_blocked.end()) {
          std::pair<std::map<uint64_t, size_t>::iterator,bool> result;
          result = most_common_blocked.insert(
              std::make_pair(blocked_tid, it->second + count));
          if (!result.second) {
            result.first->second = it->second + count;
          }
        } else {
          most_common_blocked.emplace(blocked_tid, count);
        }
        it = most_common_blocker.find(owner_tid);
        if (it != most_common_blocker.end()) {
          std::pair<std::map<uint64_t, size_t>::iterator,bool> result;
          result = most_common_blocker.insert(
              std::make_pair(owner_tid, it->second + count));
          if (!result.second) {
            result.first->second = it->second + count;
          }
        } else {
          most_common_blocker.emplace(owner_tid, count);
        }
      }
    }
    uint64_t max_tid = 0;
    size_t max_tid_count = 0;
    for (const auto& pair : most_common_blocked) {
      if (pair.second > max_tid_count) {
        max_tid = pair.first;
        max_tid_count = pair.second;
      }
    }
    if (max_tid != 0) {
      os << " sample shows most blocked tid=" << max_tid;
    }
    max_tid = 0;
    max_tid_count = 0;
    for (const auto& pair : most_common_blocker) {
      if (pair.second > max_tid_count) {
        max_tid = pair.first;
        max_tid_count = pair.second;
      }
    }
    if (max_tid != 0) {
      os << " sample shows tid=" << max_tid << " owning during this time";
    }
  }
#endif
#endif
}


Mutex::Mutex(const char* name, LockLevel level, bool recursive)
    : BaseMutex(name, level), recursive_(recursive), recursion_count_(0) {
#if V8_USE_FUTEXES
  DCHECK_EQ(0, state_.LoadRelaxed());
  DCHECK_EQ(0, num_contenders_.LoadRelaxed());
#else
  CHECK_MUTEX_CALL(pthread_mutex_init, (&mutex_, NULL));
#endif
  exclusive_owner_ = 0;
}


Mutex::~Mutex() {
#if V8_USE_FUTEXES
  if (kDebugLocking) {
    // VMThreadList shut_down_lock_ and thread_suspend_count_lock_ may hold
    // by thread while main is destroying the mutex
    // See ThreadList Unregister
    bool shutting_down = WorkerRuntime::IsShuttingDown();
    if (state_.LoadRelaxed() != 0 && !shutting_down) {
      V8_Fatal(__FILE__,
               __LINE__,
#ifdef __LP64__
               "destroying mutex with owner: %lu",
#else
               "destroying mutex with owner: %llu",
#endif
               exclusive_owner_);
    } else {
      if (exclusive_owner_ != 0 && !shutting_down) {
        V8_Fatal(__FILE__,
                 __LINE__,
                 "unexpectedly found an owner on unlocked mutex %s",
                 name_);
      }
      if (num_contenders_.LoadSequentiallyConsistent() != 0 && !shutting_down) {
        V8_Fatal(__FILE__,
                 __LINE__,
                 "unexpectedly found found a contender on mutex %s",
                 name_);
      }
    }
  }
#else
  // We can't use CHECK_MUTEX_CALL here because on
  // shutdown a suspended daemon thread may still be using locks.
  int rc = pthread_mutex_destroy(&mutex_);
  DCHECK_EQ(rc, 0);
  USE(rc);
#endif
}

void Mutex::ExclusiveLock(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  if (kDebugLocking && !recursive_) {
    AssertNotHeld(self);
  }
  if (!recursive_ || !IsExclusiveHeld(self)) {
#if V8_USE_FUTEXES
    bool done = false;
    do {
      int32_t cur_state = state_.LoadRelaxed();
      if (V8_LIKELY(cur_state == 0)) {
        // Change state from 0 to 1 and impose load/store
        // ordering appropriate for lock acquisition.
        done = state_.CompareExchangeWeakAcquire(0 /* cur_state */,
                                                 1 /* new state */);
      } else {
        // Failed to acquire, hang up.
        ScopedContentionRecorder scr(this,
                                     SafeGetTid(self),
                                     GetExclusiveOwnerTid());
        num_contenders_++;
        if (futex(state_.Address(), FUTEX_WAIT, 1, NULL, NULL, 0) != 0) {
          // EAGAIN and EINTR both indicate a spurious failure,
          // try again from the beginning. We don't use TEMP_FAILURE_RETRY
          // so we can intentionally retry to acquire the lock.
          if ((errno != EAGAIN) && (errno != EINTR)) {
            V8_Fatal(__FILE__, __LINE__,
                     "futex wait failed for %d %s", cur_state, name_);
          }
        }
        num_contenders_--;
      }
    } while (!done);
    DCHECK_EQ(state_.LoadRelaxed(), 1);
#else
    CHECK_MUTEX_CALL(pthread_mutex_lock, (&mutex_));
#endif
    DCHECK_EQ(exclusive_owner_, 0U);
    exclusive_owner_ = SafeGetTid(self);
    RegisterAsLocked(self);
  }
  recursion_count_++;
  if (kDebugLocking) {
    if (recursion_count_ != 1 && !recursive_) {
      V8_Fatal(__FILE__, __LINE__, "Unexpected recursion count on mutex: %s %d",
          name_, recursion_count_);
    }
    AssertHeld(self);
  }
}


bool Mutex::ExclusiveTryLock(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  if (kDebugLocking && !recursive_) {
    AssertNotHeld(self);
  }
  if (!recursive_ || !IsExclusiveHeld(self)) {
#if V8_USE_FUTEXES
    bool done = false;
    do {
      int32_t cur_state = state_.LoadRelaxed();
      if (cur_state == 0) {
        // Change state from 0 to 1 and impose load/store
        // ordering appropriate for lock acquisition.
        done = state_.CompareExchangeWeakAcquire(0 /* cur_state */,
                                                 1 /* new state */);
      } else {
        return false;
      }
    } while (!done);
    DCHECK_EQ(state_.LoadRelaxed(), 1);
#else
    int result = pthread_mutex_trylock(&mutex_);
    if (result == EBUSY) {
      return false;
    }
    DCHECK_EQ(result, 0);
#endif
    DCHECK_EQ(exclusive_owner_, 0U);
    exclusive_owner_ = SafeGetTid(self);
    RegisterAsLocked(self);
  }
  recursion_count_++;
  if (kDebugLocking) {
    if (recursion_count_ != 1 && !recursive_) {
      V8_Fatal(__FILE__, __LINE__, "Unexpected recursion count on mutex: %s %d",
          name_, recursion_count_);
    }
    AssertHeld(self);
  }
  return true;
}


void Mutex::ExclusiveUnlock(VMThread* self) {
  if (kDebugLocking && self != NULL && !self->IsCurrentThread()) {
    uint32_t tid1 = VMThreadList::kInvalidThreadId;
    uint32_t tid2 = VMThreadList::kInvalidThreadId;
    if (self != NULL) {
      tid1 = self->thread_id();
    }
    if (VMThread::Current() != NULL) {
      tid2 = VMThread::Current()->thread_id();
    }
    V8_Fatal(__FILE__, __LINE__,
             "%s,  level=%d self=%d VMThread::Current()=%d",
             GetName(), level_, tid1, tid2);
  }
  AssertHeld(self);
  DCHECK_NE(exclusive_owner_, 0U);
  recursion_count_--;
  if (!recursive_ || recursion_count_ == 0) {
    if (kDebugLocking) {
      if (recursion_count_ != 0 && !recursive_) {
        V8_Fatal(__FILE__, __LINE__,
                 "Unexpected recursion count on mutex: %s %d",
                 name_, recursion_count_);
      }
    }
    RegisterAsUnlocked(self);
#if V8_USE_FUTEXES
    bool done = false;
    do {
      int32_t cur_state = state_.LoadRelaxed();
      if (V8_LIKELY(cur_state == 1)) {
        // We're no longer the owner.
        exclusive_owner_ = 0;
        // Change state to 0 and impose load/store ordering
        // appropriate for lock release.
        // Note, the relaxed loads below musn't reorder before
        // the CompareExchange.
        // TODO: the ordering here is non-trivial as state is split across 3
        // fields, fix by placing a status bit into the state on contention.
        done =  state_.CompareExchangeWeakSequentiallyConsistent(
            cur_state, 0 /* new state */);
        if (V8_LIKELY(done)) {  // Spurious fail?
          // Wake a contender.
          if (V8_UNLIKELY(num_contenders_.LoadRelaxed() > 0)) {
            futex(state_.Address(), FUTEX_WAKE, 1, NULL, NULL, 0);
          }
        }
      } else {
        // Logging acquires the logging lock, avoid infinite
        // recursion in that case.
        V8_Fatal(__FILE__, __LINE__, "Unexpected state_:%d in unlock for %s",
            cur_state, name_);
      }
    } while (!done);
#else
    exclusive_owner_ = 0;
    CHECK_MUTEX_CALL(pthread_mutex_unlock, (&mutex_));
#endif
  }
}


void Mutex::Dump(std::ostream& os) {
  os << (recursive_ ? "recursive " : "non-recursive ")
      << name_
      << " level=" << static_cast<int>(level_)
      << " rec=" << recursion_count_
      << " owner=" << GetExclusiveOwnerTid() << " ";
  DumpContention(os);
}


std::ostream& operator<<(std::ostream& os, Mutex& mu) {
  mu.Dump(os);
  return os;
}


ReaderWriterMutex::ReaderWriterMutex(const char* name, LockLevel level)
    : BaseMutex(name, level)
#if V8_USE_FUTEXES
    , state_(0), num_pending_readers_(0), num_pending_writers_(0)
#endif
{  // NOLINT(whitespace/braces)
#if !V8_USE_FUTEXES
  CHECK_MUTEX_CALL(pthread_rwlock_init, (&rwlock_, NULL));
#endif
  exclusive_owner_ = 0;
}


ReaderWriterMutex::~ReaderWriterMutex() {
#if V8_USE_FUTEXES
  CHECK_EQ(state_.LoadRelaxed(), 0);
  CHECK_EQ(exclusive_owner_, 0U);
  CHECK_EQ(num_pending_readers_.LoadRelaxed(), 0);
  CHECK_EQ(num_pending_writers_.LoadRelaxed(), 0);
#else
  // We can't use CHECK_MUTEX_CALL here because on shutdown
  // a suspended daemon thread
  // may still be using locks.
  int rc = pthread_rwlock_destroy(&rwlock_);
  DCHECK_EQ(rc, 0);
  USE(rc);
#endif
}


void ReaderWriterMutex::ExclusiveLock(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  AssertNotExclusiveHeld(self);
#if V8_USE_FUTEXES
  bool done = false;
  do {
    int32_t cur_state = state_.LoadRelaxed();
    if (V8_LIKELY(cur_state == 0)) {
      // Change state from 0 to -1 and impose load/store ordering
      // appropriate for lock acquisition.
      done =  state_.CompareExchangeWeakAcquire(0 /* cur_state*/,
                                                -1 /* new state */);
    } else {
      // Failed to acquire, hang up.
      ScopedContentionRecorder scr(this,
                                   SafeGetTid(self),
                                   GetExclusiveOwnerTid());
      ++num_pending_writers_;
      if (futex(state_.Address(), FUTEX_WAIT, cur_state, NULL, NULL, 0) != 0) {
        // EAGAIN and EINTR both indicate a spurious failure, try
        // again from the beginning. We don't use TEMP_FAILURE_RETRY
        // so we can intentionally retry to acquire the lock.
        if ((errno != EAGAIN) && (errno != EINTR)) {
          V8_Fatal(__FILE__, __LINE__, "futex wait failed for %s", name_);
        }
      }
      --num_pending_writers_;
    }
  } while (!done);
  DCHECK_EQ(state_.LoadRelaxed(), -1);
#else
  CHECK_MUTEX_CALL(pthread_rwlock_wrlock, (&rwlock_));
#endif
  DCHECK_EQ(exclusive_owner_, 0U);
  exclusive_owner_ = SafeGetTid(self);
  RegisterAsLocked(self);
  AssertExclusiveHeld(self);
}


void ReaderWriterMutex::ExclusiveUnlock(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  AssertExclusiveHeld(self);
  RegisterAsUnlocked(self);
  DCHECK_NE(exclusive_owner_, 0U);
#if V8_USE_FUTEXES
  bool done = false;
  do {
    int32_t cur_state = state_.LoadRelaxed();
    if (V8_LIKELY(cur_state == -1)) {
      // We're no longer the owner.
      exclusive_owner_ = 0;
      // Change state from -1 to 0 and impose load/store ordering
      // appropriate for lock release.
      // Note, the relaxed loads below musn't reorder
      // before the CompareExchange.
      // TODO: the ordering here is non-trivial as state is split across 3
      // fields, fix by placing a status bit into the state on contention.
      done =  state_.CompareExchangeWeakSequentiallyConsistent(
          -1 /* cur_state*/, 0 /* new state */);
      if (V8_LIKELY(done)) {  // Weak CAS may fail spuriously.
        // Wake any waiters.
        if (V8_UNLIKELY(num_pending_readers_.LoadRelaxed() > 0 ||
                     num_pending_writers_.LoadRelaxed() > 0)) {
          futex(state_.Address(), FUTEX_WAKE, -1, NULL, NULL, 0);
        }
      }
    } else {
      V8_Fatal(__FILE__, __LINE__,
               "Unexpected state_:%d for %s", cur_state, name_);
    }
  } while (!done);
#else
  exclusive_owner_ = 0;
  CHECK_MUTEX_CALL(pthread_rwlock_unlock, (&rwlock_));
#endif
}


#if HAVE_TIMED_RWLOCK
bool ReaderWriterMutex::ExclusiveLockWithTimeout(
    VMThread* self, const base::TimeDelta& rel_time) {
  DCHECK(self == NULL || self->IsCurrentThread());
#if V8_USE_FUTEXES
  base::Time now = base::Time::Now();
  base::Time end_time = now + rel_time;

  bool done = false;
  do {
    int32_t cur_state = state_.LoadRelaxed();
    if (cur_state == 0) {
      // Change state from 0 to -1 and impose load/store ordering
      // appropriate for lock acquisition.
      done =  state_.CompareExchangeWeakAcquire(0 /* cur_state */,
                                                -1 /* new state */);
    } else {
      // Failed to acquire, hang up.
      base::Time now_abs = base::Time::Now();
      base::TimeDelta delta_time = end_time - now_abs;
      if (delta_time.InMicroseconds() < 0) {
        return false;  // Timed out.
      }
      timespec rel_ts = delta_time.ToTimespec();

      ScopedContentionRecorder scr(this,
                                   SafeGetTid(self),
                                   GetExclusiveOwnerTid());
      ++num_pending_writers_;
      if (futex(state_.Address(), FUTEX_WAIT,
                cur_state, &rel_ts, NULL, 0) != 0) {
        if (errno == ETIMEDOUT) {
          --num_pending_writers_;
          return false;  // Timed out.
        } else if ((errno != EAGAIN) && (errno != EINTR)) {
          // EAGAIN and EINTR both indicate a spurious failure,
          // recompute the relative time out from now and try again.
          // We don't use TEMP_FAILURE_RETRY so we can recompute rel_ts;
          V8_Fatal(__FILE__, __LINE__, "timed futex wait failed for %s", name_);
        }
      }
      --num_pending_writers_;
    }
  } while (!done);
#else
  base::Time now = base::Time::Now();
  base::Time end_time = now + rel_time;
  timespec ts = end_time.ToTimespec();

  int result = pthread_rwlock_timedwrlock(&rwlock_, &ts);
  if (result == ETIMEDOUT) {
    return false;
  }
  if (result != 0) {
    errno = result;
    V8_Fatal(__FILE__, __LINE__,
             "pthread_rwlock_timedwrlock failed for %s", name_);
  }
#endif
  exclusive_owner_ = SafeGetTid(self);
  RegisterAsLocked(self);
  AssertSharedHeld(self);
  return true;
}
#endif


#if V8_USE_FUTEXES
void ReaderWriterMutex::HandleSharedLockContention(
    VMThread* self, int32_t cur_state) {
  // Owner holds it exclusively, hang up.
  ScopedContentionRecorder scr(this, GetExclusiveOwnerTid(), SafeGetTid(self));
  ++num_pending_readers_;
  if (futex(state_.Address(), FUTEX_WAIT, cur_state, NULL, NULL, 0) != 0) {
    if (errno != EAGAIN) {
      V8_Fatal(__FILE__, __LINE__, "futex wait failed for %s", name_);
    }
  }
  --num_pending_readers_;
}
#endif


bool ReaderWriterMutex::SharedTryLock(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
#if V8_USE_FUTEXES
  bool done = false;
  do {
    int32_t cur_state = state_.LoadRelaxed();
    if (cur_state >= 0) {
      // Add as an extra reader and impose load/store
      // ordering appropriate for lock acquisition.
      done =  state_.CompareExchangeWeakAcquire(cur_state, cur_state + 1);
    } else {
      // Owner holds it exclusively.
      return false;
    }
  } while (!done);
#else
  int result = pthread_rwlock_tryrdlock(&rwlock_);
  if (result == EBUSY) {
    return false;
  }
  if (result != 0) {
    errno = result;
    V8_Fatal(__FILE__, __LINE__, "pthread_mutex_trylock failed for %s", name_);
  }
#endif
  RegisterAsLocked(self);
  AssertSharedHeld(self);
  return true;
}


bool ReaderWriterMutex::IsSharedHeld(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  bool result;
  if (V8_UNLIKELY(self == NULL)) {  // Handle unattached threads.
    result = IsExclusiveHeld(self);  // TODO: a better best effort here.
  } else {
    result = (self->held_mutex(level_) == this);
  }
  return result;
}


void ReaderWriterMutex::Dump(std::ostream& os) {
  os << name_
      << " level=" << static_cast<int>(level_)
      << " owner=" << static_cast<int>(GetExclusiveOwnerTid())
#if V8_USE_FUTEXES
      << " state=" << state_
      << " num_pending_writers="
      << static_cast<int>(num_pending_writers_.LoadSequentiallyConsistent())
      << " num_pending_readers="
      << static_cast<int>(num_pending_readers_.LoadSequentiallyConsistent())
#endif
      << " ";
  DumpContention(os);
}


std::ostream& operator<<(std::ostream& os, ReaderWriterMutex& mu) {
  mu.Dump(os);
  return os;
}


ConditionVariable::ConditionVariable(const char* name, Mutex& guard)
    : name_(name), guard_(guard) {
#if V8_USE_FUTEXES
  DCHECK_EQ(0, sequence_.LoadRelaxed());
  num_waiters_ = 0;
#else
  pthread_condattr_t cond_attrs;
  CHECK_MUTEX_CALL(pthread_condattr_init, (&cond_attrs));
#if !defined(__APPLE__)
  // Apple doesn't have CLOCK_MONOTONIC or pthread_condattr_setclock.
  CHECK_MUTEX_CALL(pthread_condattr_setclock, (&cond_attrs, CLOCK_MONOTONIC));
#endif
  CHECK_MUTEX_CALL(pthread_cond_init, (&cond_, &cond_attrs));
#endif
}


ConditionVariable::~ConditionVariable() {
#if V8_USE_FUTEXES
  if (num_waiters_!= 0) {
#if DEBUG
    bool shutting_down = WorkerRuntime::IsShuttingDown();
    if (!shutting_down) {
      V8_Fatal(__FILE__, __LINE__,
          "ConditionVariable::~ConditionVariable for %s called with %d waiters.",
          name_, num_waiters_);
    }
#endif
  }
#else
  // We can't use CHECK_MUTEX_CALL here because on shutdown
  // a suspended daemon thread may still be using condition variables.
  int rc = pthread_cond_destroy(&cond_);
  DCHECK_EQ(rc, 0);
  USE(rc)
#endif
}


void ConditionVariable::Broadcast(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  // TODO: enable below, there's a race in thread creation that causes
  // false failures currently. guard_.AssertExclusiveHeld(self);
  DCHECK_EQ(guard_.GetExclusiveOwnerTid(), SafeGetTid(self));
#if V8_USE_FUTEXES
  if (num_waiters_ > 0) {
    sequence_++;  // Indicate the broadcast occurred.
    bool done = false;
    do {
      int32_t cur_sequence = sequence_.LoadRelaxed();
      // Requeue waiters onto mutex. The waiter holds the contender count on
      // the mutex high ensuring mutex unlocks will awaken the
      // requeued waiter thread.
      done = futex(sequence_.Address(), FUTEX_CMP_REQUEUE, 0,
                   reinterpret_cast<const timespec*>(
                      std::numeric_limits<int32_t>::max()),
                   guard_.state_.Address(), cur_sequence) != -1;
      if (!done) {
        if (errno != EAGAIN) {
          V8_Fatal(__FILE__, __LINE__,
                   "futex cmp requeue failed for %s", name_);
        }
      }
    } while (!done);
  }
#else
  CHECK_MUTEX_CALL(pthread_cond_broadcast, (&cond_));
#endif
}

void ConditionVariable::Signal(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  guard_.AssertExclusiveHeld(self);
#if V8_USE_FUTEXES
  if (num_waiters_ > 0) {
     sequence_++;  // Indicate a signal occurred.
    // Futex wake 1 waiter who will then come and in contend on mutex.
     // It'd be nice to requeue them
    // to avoid this, however, requeueing can only move all waiters.
    int num_woken = futex(sequence_.Address(), FUTEX_WAKE, 1, NULL, NULL, 0);
    // Check something was woken or else we changed sequence_
    // before they had chance to wait.
    CHECK((num_woken == 0) || (num_woken == 1));
  }
#else
  CHECK_MUTEX_CALL(pthread_cond_signal, (&cond_));
#endif
}


void ConditionVariable::Wait(VMThread* self) {
  guard_.CheckSafeToWait(self);
  WaitHoldingLocks(self);
}


void ConditionVariable::WaitHoldingLocks(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  guard_.AssertExclusiveHeld(self);
  unsigned int old_recursion_count = guard_.recursion_count_;
#if V8_USE_FUTEXES
  num_waiters_++;
  // Ensure the Mutex is contended so that requeued threads are awoken.
  guard_.num_contenders_++;
  guard_.recursion_count_ = 1;
  int32_t cur_sequence = sequence_.LoadRelaxed();
  guard_.ExclusiveUnlock(self);
  if (futex(sequence_.Address(), FUTEX_WAIT,
            cur_sequence, NULL, NULL, 0) != 0) {
    // Futex failed, check it is an expected error.
    // EAGAIN == EWOULDBLK, so we let the caller try again.
    // EINTR implies a signal was sent to this thread.
    if ((errno != EINTR) && (errno != EAGAIN)) {
      V8_Fatal(__FILE__, __LINE__, "futex wait failed for %s", name_);
    }
  }
  guard_.ExclusiveLock(self);
  CHECK_GE(num_waiters_, 0);
  num_waiters_--;
  // We awoke and so no longer require awakes from the guard_'s unlock.
  CHECK_GE(guard_.num_contenders_.LoadRelaxed(), 0);
  guard_.num_contenders_--;
#else
  uint64_t old_owner = guard_.exclusive_owner_;
  guard_.exclusive_owner_ = 0;
  guard_.recursion_count_ = 0;
  CHECK_MUTEX_CALL(pthread_cond_wait, (&cond_, &guard_.mutex_));
  guard_.exclusive_owner_ = old_owner;
#endif
  guard_.recursion_count_ = old_recursion_count;
}


bool ConditionVariable::TimedWait(
    VMThread* self, const base::TimeDelta& rel_time) {
  DCHECK(self == NULL || self->IsCurrentThread());
  bool timed_out = false;
  guard_.AssertExclusiveHeld(self);
  guard_.CheckSafeToWait(self);
  unsigned int old_recursion_count = guard_.recursion_count_;
#if V8_USE_FUTEXES
  timespec rel_ts = rel_time.ToTimespec();

  num_waiters_++;
  // Ensure the Mutex is contended so that requeued threads are awoken.
  guard_.num_contenders_++;
  guard_.recursion_count_ = 1;
  int32_t cur_sequence = sequence_.LoadRelaxed();
  guard_.ExclusiveUnlock(self);
  if (futex(sequence_.Address(), FUTEX_WAIT,
            cur_sequence, &rel_ts, NULL, 0) != 0) {
    if (errno == ETIMEDOUT) {
      // Timed out we're done.
      timed_out = true;
    } else if ((errno == EAGAIN) || (errno == EINTR)) {
      // A signal or ConditionVariable::Signal/Broadcast has come in.
    } else {
      V8_Fatal(__FILE__, __LINE__, "futex wait failed for %s", name_);
    }
  }
  guard_.ExclusiveLock(self);
  CHECK_GE(num_waiters_, 0);
  num_waiters_--;
  // We awoke and so no longer require awakes from the guard_'s unlock.
  CHECK_GE(guard_.num_contenders_.LoadRelaxed(), 0);
  guard_.num_contenders_--;
#else
  uint64_t old_owner = guard_.exclusive_owner_;
  guard_.exclusive_owner_ = 0;
  guard_.recursion_count_ = 0;

  base::Time end_time = base::Time::Now() + rel_time;
  timespec ts = end_time.ToTimespec();

  int rc = TEMP_FAILURE_RETRY(pthread_cond_timedwait(&cond_,
                                                     &guard_.mutex_,
                                                     &ts));
  if (rc == ETIMEDOUT) {
    timed_out = true;
  } else if (rc != 0) {
    errno = rc;
    PLOG(FATAL) << "TimedWait failed for " << name_;
  }
  guard_.exclusive_owner_ = old_owner;
#endif
  guard_.recursion_count_ = old_recursion_count;
  return timed_out;
}


void Locks::Init() {
}


void Locks::InitConditions() {
}

SafepointMutexLock::SafepointMutexLock(VMThread* self, Mutex& mu)
  : self_(self), mu_(mu) {
  // JSMT TODO: Allow others to suspend self when self is prepared to wait on a
  // lock and this requires this happen at "safe point". Change thread state to
  // kSuspended before the lock call and change it to kRunnable when done.
  // Sunzhe said it's safe in V8 when we wait on a global lock though it maybe not
  // a safepoint. In phase I, we assume lock users follow the rule that only lock
  // at "safe point".
  // base::OS::Print("%p: SafepointMutexLock: %s\n", (void*)self_, mu_.GetName());
  self->AssertCurrentThread();
  // NOTE(JSMT) : enable this to check whther the place is safe currently.
  should_lock_ = !self->IsThreadSafe();
  if (should_lock_) {
    DCHECK(AllowHeapAllocation::IsAllowed());
    ThreadStateChangeScope tsc(self_, kWait);
    mu_.ExclusiveLock(self_);
  }
}


}  // namespace sync
}  // namespace internal
}  // namespace v8
