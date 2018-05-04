/**
 * {{{ Copyright (C) 2015 The YunOS Project. All rights reserved. }}}
 */

#ifndef THREAD_VMTHREAD_INL_H_
#define THREAD_VMTHREAD_INL_H_

#include "src/heap/heap.h"
#include "src/isolate.h"
#include "src/objects-inl.h"
#include "src/thread/vmthread.h"
#include "src/thread/sync/mutex-inl.h"
#include "src/objects/scope-info.h"
#include "src/thread/worker-runtime.h"

namespace v8 {
namespace internal {

inline VMThread* VMThread::Current() {
  Isolate* current = Isolate::Current();
  return current != nullptr ? current->vmthread() : nullptr;
}

inline void VMThread::AssertMutatorRunnable() {
#ifdef DEBUG
  CHECK(state() == kUnInitialize || state() == kRunnable ||
        thread_list()->in_suspend_all_scope_ || !IsJSThread());
#endif
}

inline void VMThread::AssertCurrentThread() {
#ifdef DEBUG
  Isolate* current = Isolate::Current();
  CHECK(isolate() == current || single_thread_mode() ||
        current == nullptr ||
       (isolate()->is_main_isolate() && current->is_main_isolate()));
#endif
}

inline bool VMThread::IsCurrentThread() {
  Isolate* current = Isolate::Current();
  return isolate() == current || single_thread_mode() ||
        current == nullptr ||
       (isolate()->is_main_isolate() && current->is_main_isolate());
}

inline bool VMThread::IsThreadSafe() {
  return single_thread_mode() || IsInSuspendAll();
}

inline bool VMThread::IsJSThread() {
  return isolate()->worker_runtime()->IsJSThread();
}

inline Isolate* VMThread::isolate() {
  return reinterpret_cast<Isolate*>(
      reinterpret_cast<intptr_t>(this) -
      reinterpret_cast<size_t>(reinterpret_cast<Isolate*>(16)->vmthread()) + 16);
}

inline void VMThread::TransitionFromSuspendedToRunnable() {
  AssertCurrentThread();
  DCHECK_NE(flags_and_state_.i.as_state, kRunnable);
  union FlagsAndState old_flags_and_state;
  VMThreadList* threads = thread_list();
  do {
    old_flags_and_state.as_int = flags_and_state_.as_int;
    if (V8_LIKELY(old_flags_and_state.i.as_flags == 0)) {
      union FlagsAndState new_flags_and_state;
      new_flags_and_state.as_int = old_flags_and_state.as_int;
      new_flags_and_state.i.as_state = kRunnable;
      // CAS the value with a memory barrier.
      if (V8_LIKELY(flags_and_state_.as_atomic_int.CompareExchangeWeakAcquire(
                 old_flags_and_state.as_int, new_flags_and_state.as_int))) {
        // we will change to Runnable, hold the share
        // of the mutator_lock_.
        threads->mutator_lock_.RegisterAsLocked(this);
        threads->mutator_lock_.AssertSharedHeld(this);
        break;
      }
    } else if ((old_flags_and_state.i.as_flags & kActiveSuspendBarrier) != 0) {
      PassActiveSuspendBarriers();
    } else if ((old_flags_and_state.i.as_flags & kCheckpointRequest) != 0) {
      UNREACHABLE();
    } else if ((old_flags_and_state.i.as_flags & kSuspendRequest) != 0) {
      sync::MutexLock mu(this, threads->thread_suspend_count_lock_);
      old_flags_and_state.as_int = flags_and_state_.as_int;
      while ((old_flags_and_state.i.as_flags & kSuspendRequest) != 0) {
        threads->resume_cond_.Wait(this);
        old_flags_and_state.as_int = flags_and_state_.as_int;
      }
      CHECK_EQ(suspend_count_, 0);
    }
  } while(true);
}


inline void VMThread::TransitionFromRunnableToSuspended(VMThreadState new_state) {
  AssertCurrentThread();
  DCHECK_NE(new_state, kRunnable);

  VMThreadList* threads = thread_list();
  union FlagsAndState old_flags_and_state;
  union FlagsAndState new_flags_and_state;

  while (true) {
    old_flags_and_state.as_int = flags_and_state_.as_int;
    if ((old_flags_and_state.i.as_flags & kCheckpointRequest) != 0) {
      RunCheckpointFunction();
      continue;
    }
    new_flags_and_state.as_int = old_flags_and_state.as_int;
    new_flags_and_state.i.as_state = new_state;
    // CAS the value with a memory ordering.
    bool done = flags_and_state_.as_atomic_int.CompareExchangeWeakRelease(
           old_flags_and_state.as_int, new_flags_and_state.as_int);
    if (V8_LIKELY(done)) {
      break;
    }
  }
  // we will change to Runnable, hold the share
  // of the mutator_lock_.
  threads->mutator_lock_.AssertSharedHeld(this);
  threads->mutator_lock_.RegisterAsUnlocked(this);
  while (true) {
    uint32_t current_flags = flags_and_state_.i.as_flags;
    if ((current_flags & (kActiveSuspendBarrier | kCheckpointRequest)) == 0) {
      break;
    } else if ((current_flags & kActiveSuspendBarrier) != 0) {
      PassActiveSuspendBarriers();
    } else {
      UNREACHABLE();
    }
  }
}


inline void VMThread::ModifySuspendCount(VMThread* self, int32_t delta,
                        sync::Barrier* suspend_barrier) {
  VMThreadList* threads = thread_list();
  threads->thread_suspend_count_lock_.AssertHeld(self);
  if (suspend_barrier != NULL) {
      while (true) {
        if (V8_LIKELY(ModifySuspendCountInternal(+1, suspend_barrier))) {
          break;
        } else {
          threads->thread_suspend_count_lock_.ExclusiveUnlock(self);
          base::OS::Sleep(base::TimeDelta::FromNanoseconds(100000));
          threads->thread_suspend_count_lock_.ExclusiveLock(self);
        }
      }
  } else {
    ModifySuspendCountInternal(delta, suspend_barrier);
  }
}

inline StackGuard* VMThread::stack_guard() {
  return isolate()->stack_guard();
}

}  // namespace internal
}  // namespace v8

#endif  // THREAD_VMTHREAD_INL_H_
