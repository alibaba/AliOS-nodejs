
// The source is based on Android Runtime VM.
// https://source.android.com/source/downloading.html

#ifndef THREAD_MUTEX_INL_H_
#define THREAD_MUTEX_INL_H_

#include "src/thread/sync/mutex.h"
#include "src/base/atomicops.h"
#include "src/base/platform/platform.h"
#include "src/thread/vmthread-inl.h"

#if V8_USE_FUTEXES
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#ifndef SYS_futex
#define SYS_futex __NR_futex
#endif
#endif  // V8_USE_FUTEXES

#define CHECK_MUTEX_CALL(call, args) \
  do { \
    int rc = call args; \
    DCHECK_EQ(rc, 0);   \
    USE(rc);            \
  } while (false)


namespace v8 {
namespace internal {
namespace sync {

#if V8_USE_FUTEXES
static inline int futex(volatile int *uaddr, int op, int val,
                        const struct timespec *timeout,
                        volatile int *uaddr2, int val3) {
  return static_cast<int>(syscall(SYS_futex, uaddr, op, val,
                                  timeout, uaddr2, val3));
}
#endif  // V8_USE_FUTEXES


static inline uint64_t SafeGetTid(VMThread* self) {
  if (self != NULL) {
    return static_cast<uint64_t>(self->thread_id());
  } else {
    return static_cast<uint64_t>(base::OS::GetCurrentThreadId());
  }
}


static inline void CheckUnattachedVMThread(LockLevel level)
    NO_THREAD_SAFETY_ANALYSIS {
  // The check below enumerates the cases where we expect not to
  // be able to sanity check locks on a thread. Lock checking is
  // disabled to avoid deadlock when checking shutdown lock.
  // TODO: tighten this check.
  if (kDebugLocking) {
    CHECK(level == kDefaultMutexLevel);
  }
}


inline void BaseMutex::RegisterAsLocked(VMThread* self) {
  if (V8_UNLIKELY(self == NULL)) {
    CheckUnattachedVMThread(level_);
    return;
  }

  if (kDebugLocking) {
    // Check if a bad Mutex of this level or lower is held.
    for (int i = level_; i >= 0; --i) {
      BaseMutex* held_mutex = self->held_mutex(static_cast<LockLevel>(i));
      CHECK(held_mutex == NULL);
    }
  }
  // Don't record monitors as they are outside the scope of analysis.
  // They may be inspected off of the monitor list.
  if (level_ != kDefaultMutexLevel) {
    self->set_held_mutex(level_, this);
  }
}


inline void BaseMutex::RegisterAsUnlocked(VMThread* self) {
  if (V8_UNLIKELY(self == NULL)) {
    CheckUnattachedVMThread(level_);
    return;
  }

  if (level_ != kDefaultMutexLevel) {
    if (kDebugLocking) {
      CHECK(self->held_mutex(level_) == this);
    }
    self->set_held_mutex(level_, NULL);
  }
}


inline void ReaderWriterMutex::SharedLock(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
#if V8_USE_FUTEXES
  bool done = false;
  do {
    int32_t cur_state = state_.LoadRelaxed();
    if (V8_LIKELY(cur_state >= 0)) {
      // Add as an extra reader.
      done = state_.CompareExchangeWeakAcquire(cur_state, cur_state + 1);
    } else {
      HandleSharedLockContention(self, cur_state);
    }
  } while (!done);
#else
  CHECK_MUTEX_CALL(pthread_rwlock_rdlock, (&rwlock_));
#endif
  DCHECK(exclusive_owner_ == 0U || exclusive_owner_ == -1U);
  RegisterAsLocked(self);
  AssertSharedHeld(self);
}


inline void ReaderWriterMutex::SharedUnlock(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  DCHECK(exclusive_owner_ == 0U || exclusive_owner_ == -1U);
  AssertSharedHeld(self);
  RegisterAsUnlocked(self);
#if V8_USE_FUTEXES
  bool done = false;
  do {
    int32_t cur_state = state_.LoadRelaxed();
    if (V8_LIKELY(cur_state > 0)) {
      // Reduce state by 1 and impose lock release load/store ordering.
      // Note, the relaxed loads below musn't reorder before the
      // CompareExchange.
      // TODO: the ordering here is non-trivial as state is split across
      // 3 fields, fix by placing a status bit into the state on contention.

      done = state_.CompareExchangeWeakSequentiallyConsistent(cur_state,
                                                              cur_state - 1);
      if (done && (cur_state - 1) == 0) {
        if (num_pending_writers_.LoadRelaxed() > 0 ||
            num_pending_readers_.LoadRelaxed() > 0) {
          // Wake any exclusive waiters as there are now no readers.
          futex(state_.Address(), FUTEX_WAKE, -1, NULL, NULL, 0);
        }
      }
    } else {
      V8_Fatal(__FILE__, __LINE__, "Unexpected state_:%d for %s",
               cur_state, name_);
    }
  } while (!done);
#else
  CHECK_MUTEX_CALL(pthread_rwlock_unlock, (&rwlock_));
#endif
}


inline bool Mutex::IsExclusiveHeld(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  bool result = (GetExclusiveOwnerTid() == SafeGetTid(self));
  if (kDebugLocking) {
    // Sanity debug check that if we think it is locked
    // we have it in our held mutexes.
    if (result && self != NULL && level_ != kDefaultMutexLevel) {
      CHECK_EQ(self->held_mutex(level_), this);
    }
  }
  return result;
}


inline uint64_t Mutex::GetExclusiveOwnerTid() {
  return exclusive_owner_;
}


inline bool ReaderWriterMutex::IsExclusiveHeld(VMThread* self) {
  DCHECK(self == NULL || self->IsCurrentThread());
  bool result = (GetExclusiveOwnerTid() == SafeGetTid(self));
  if (kDebugLocking) {
    // Sanity that if the pthread thinks we own the lock the VMThread agrees.
    if (self != NULL && result)  {
      CHECK(level_ != kDefaultMutexLevel);
      CHECK_EQ(self->held_mutex(level_), this);
    }
  }
  return result;
}


inline uint64_t ReaderWriterMutex::GetExclusiveOwnerTid() {
#if V8_USE_FUTEXES
  int32_t state = state_.LoadRelaxed();
  if (state == 0) {
    return 0;  // No owner.
  } else if (state > 0) {
    return -1;  // Shared.
  } else {
    return exclusive_owner_;
  }
#else
  return exclusive_owner_;
#endif
}

}  // namespace sync
}  // namespace internal
}  // namespace v8

#endif  // THREAD_MUTEX_INL_H_
