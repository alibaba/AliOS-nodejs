/**
 * {{{ Copyright (C) 2016 The YunOS Project. All rights reserved. }}}
 */
#include "src/thread/sync/barrier.h"
#include "src/thread/sync/mutex-inl.h"
#include "src/base/platform/platform.h"
#include "src/base/platform/time.h"

namespace v8 {
namespace internal {
namespace sync {

void Barrier::Wait() {
  base::Time start_time = base::Time::Now();
  base::Time end_time =
    start_time + base::TimeDelta::FromMilliseconds(10 * 1000);
  timespec wait_timeout = end_time.ToTimespec();
  while (true) {
    int32_t cur_val = pass_barrier_.LoadRelaxed();
    if (V8_LIKELY(cur_val > 0)) {
#if V8_USE_FUTEXES
      if (sync::futex(pass_barrier_.Address(),
            FUTEX_WAIT, cur_val, &wait_timeout, NULL, 0) != 0) {
        if ((errno != EAGAIN) && (errno != EINTR)) {
          if (errno == ETIMEDOUT) {
#if DEBUG
            V8_Fatal(__FILE__, __LINE__, "futex wait timeout after 10s.");
#endif
          } else {
            V8_Fatal(__FILE__, __LINE__, "futex wait failed");
          }
        }
      }
#else
      sched_yield();
      // Spin wait. This is likely to be slow, but on most architecture
      // V8_USE_FUTEXES is set.
#endif
    } else {
      cur_val = pass_barrier_.LoadRelaxed();
      CHECK_EQ(cur_val, 0);
      break;
    }
  }
}


void Barrier::PassCount(int32_t delta) {
  bool done = false;
  do {
    int32_t cur_val = pass_barrier_.LoadRelaxed();
    // Reduce value by 1.
    done = pass_barrier_.CompareExchangeWeakRelaxed(
        cur_val, cur_val + delta);
#if V8_USE_FUTEXES
    if (done && (cur_val + delta) == 0) {  // Weak CAS may fail spuriously.
      sync::futex(pass_barrier_.Address(),
          FUTEX_WAKE, -1, NULL, NULL, 0);
    }
#endif
  } while (!done);
}

}  // namespace sync
}  // namespace internal
}  // namespace v8
