/**
 * {{{ Copyright (C) 2016 The YunOS Project. All rights reserved. }}}
 */
#ifndef THREAD_SYNC_SPIN_LOCK_H_
#define THREAD_SYNC_SPIN_LOCK_H_

#include <stdint.h>
#include "src/base/macros.h"
#include "src/thread/sync/atomic.h"

namespace v8 {
namespace internal {
namespace sync {

class SpinLock {
 public:
  static const int32_t kStateUnlock = 0;
  static const int32_t kStateLocked = 1;
  explicit SpinLock() : state_(kStateUnlock) {}
  void Lock() {
    do {
      if (V8_UNLIKELY(state_.LoadRelaxed() != kStateUnlock)) {
        sched_yield();
      }
    } while(!state_.CompareExchangeWeakAcquire(kStateUnlock, kStateLocked));
  }

  void Unlock() {
    state_.StoreRelease(kStateUnlock);
  }

 private:
  AtomicInteger state_;
  DISALLOW_COPY_AND_ASSIGN(SpinLock);
};

class SpinLockScope {
 public:
  explicit SpinLockScope(SpinLock& spin_lock)
           : spin_lock_(spin_lock) {
    spin_lock_.Lock();
  }

  ~SpinLockScope() {
    spin_lock_.Unlock();
  }
 private:
  SpinLock& spin_lock_;
  DISALLOW_COPY_AND_ASSIGN(SpinLockScope);
};

}  // namespace sync
}  // namespace internal
}  // namespace v8
#endif  // THREAD_SYNC_SPIN_LOCK_H_
