/**
 * {{{ Copyright (C) 2016 The YunOS Project. All rights reserved. }}}
 */

#ifndef THREAD_THREAD_STATE_CHANGE_INL_H_
#define THREAD_THREAD_STATE_CHANGE_INL_H_

#include "src/thread/thread-state-change.h"
#include "src/thread/vmthread-inl.h"
#include "src/api.h"

namespace v8 {
namespace internal {

inline ThreadStateChangeScope::ThreadStateChangeScope(
        VMThread* self, VMThreadState new_state)
        : self_(self), state_(new_state) {
  old_state_ = self_->state();
  if (old_state_ != new_state) {
    if (new_state == kRunnable) {
      self_->TransitionFromSuspendedToRunnable();
    } else if (old_state_ == kRunnable) {
      self_->TransitionFromRunnableToSuspended(new_state);
    } else {
      self_->set_state(new_state);
    }
  }
}

inline ThreadStateChangeScope::~ThreadStateChangeScope() {
  if (old_state_ != state_) {
    if (old_state_ == kRunnable) {
      self_->TransitionFromSuspendedToRunnable();
    } else if (state_ == kRunnable) {
      self_->TransitionFromRunnableToSuspended(old_state_);
    } else {
      self_->set_state(old_state_);
    }
  }
}

inline ApiObjectAccessScope::ApiObjectAccessScope()
       : isolate_(Isolate::Current()) {
  old_state_ = EnterSafe(isolate_);
}

inline ApiObjectAccessScope::ApiObjectAccessScope(Isolate* isolate)
       : isolate_(isolate) {
  old_state_ = Enter(isolate);
}

inline ApiObjectAccessScope::~ApiObjectAccessScope() {
  Exit(isolate_, old_state_);
}

inline ApiObjectAccessScope::ApiObjectAccessScope(VMThread* self)
    : ApiObjectAccessScope(self->isolate()) {}

inline VMThreadState ApiObjectAccessScope::EnterSafe(Isolate* isolate) {
  VMThread* self = isolate->vmthread();
  VMThreadState old_state = self->state();
  if (old_state != kRunnable && !self->IsInSuspendAll()) {
    self->TransitionFromSuspendedToRunnable();
  }
  return old_state;
}

inline VMThreadState ApiObjectAccessScope::Enter(Isolate* isolate) {
  // current isolate is unsafe
  if (isolate->IsProxyIsolate()) {
    Isolate* current = Isolate::Current();
    if (current != isolate) {
      DCHECK(current->vmthread()->IsInSuspendAll());
      return isolate->vmthread()->state();
    }
  }
  return EnterSafe(isolate);
}

inline void ApiObjectAccessScope::Exit(Isolate* isolate, VMThreadState old_state) {
  VMThread* self = isolate->vmthread();
  VMThreadState state = self->state();
  if (state != old_state) {
    DCHECK(old_state != kRunnable);
    self->TransitionFromRunnableToSuspended(old_state);
  }
}

inline ThreadSuspensionScope::ThreadSuspensionScope(VMThread* self)
  : self_(self) {
  self_->TransitionFromRunnableToSuspended(kSuspended);
}

inline ThreadSuspensionScope::~ThreadSuspensionScope() {
  self_->TransitionFromSuspendedToRunnable();
}

inline PauseScope::PauseScope(VMThread* self)
  : thread_list_(self->thread_list()),
    tsc_(self, kSuspended) {
  thread_list_->SuspendAll();
}

inline PauseScope::~PauseScope() {
  thread_list_->ResumeAll();
}

}  // namespace internal
}  // namespace v8
#endif  // THREAD_THREAD_STATE_CHANGE_INL_H_
