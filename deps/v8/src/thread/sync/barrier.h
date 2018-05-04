/**
 * {{{ Copyright (C) 2016 The YunOS Project. All rights reserved. }}}
 */
#ifndef THREAD_SYNC_BARRIER_H_
#define THREAD_SYNC_BARRIER_H_

#include <stdint.h>
#include "src/base/macros.h"
#include "src/thread/sync/atomic.h"

namespace v8 {
namespace internal {
namespace sync {

class Barrier {
 public:
  Barrier() : pass_barrier_(0) {}
  explicit Barrier(int32_t count) : pass_barrier_(count) {}
  void Wait();

  void Pass() {
    PassCount(-1);
  }

  void PassNoWeak() {
    pass_barrier_.FetchAndSubSequentiallyConsistent(1);
  }

  void Init(int32_t count) {
    pass_barrier_.StoreRelaxed(count);
  }

  void Increment(int32_t delta) {
    PassCount(delta);
    Wait();
  }

 private:
  void PassCount(int32_t delta);
  AtomicInteger pass_barrier_;
};

}  // namespace sync
}  // namespace internal
}  // namespace v8
#endif  // THREAD_SYNC_BARRIER_H_
