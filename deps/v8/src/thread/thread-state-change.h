/**
 * {{{ Copyright (C) 2016 The YunOS Project. All rights reserved. }}}
 */

#ifndef THREAD_THREAD_STATE_CHANGE_H_
#define THREAD_THREAD_STATE_CHANGE_H_
#include "src/base/macros.h"

namespace v8 {
namespace internal {

class VMThread;
class VMThreadList;

enum VMThreadFlag {
  // set and clear by other thread.
  // may wait for other threads.
  kSuspendRequest   = 1,
  // set by other thread, self clears.
  // notify the thread which set this barrier after passing barrier.
  // and continue execuation.
  kActiveSuspendBarrier = 1 << 1,
  // set by other thread, self clears.
  // won't notify and wait the thread which set this checkpointer-requirest.
  // after execution the checkpointer-callback function.
  kCheckpointRequest = 1 << 2,
};

enum VMThreadState {
  // all these states are set and clear by self.
  kUnInitialize = 0,
  // Note: it only can be set through 'TransitionFromSuspendedToRunnable'.
  kRunnable = 1,
  kSuspended = 2,
  kNative = 3,
  kWait = 4,  // Wait for lock in VM
  kTerminated = 5,
};

class ThreadStateChangeScope {
 public:
  ThreadStateChangeScope(VMThread* self, VMThreadState new_state);

  ~ThreadStateChangeScope();

 private:
  VMThread* self_;
  VMThreadState state_;
  VMThreadState old_state_;

  DISALLOW_COPY_AND_ASSIGN(ThreadStateChangeScope);
};

//Transform thread to runnable
class ApiObjectAccessScope {
 public:
  ApiObjectAccessScope();
  explicit ApiObjectAccessScope(Isolate* isolate);
  explicit ApiObjectAccessScope(VMThread* self);
  ~ApiObjectAccessScope();

  static VMThreadState Enter(Isolate* isolate);
  static VMThreadState EnterSafe(Isolate* isolate);
  static void Exit(Isolate* isolate, VMThreadState old_state);

 private:
  Isolate* isolate_;
  VMThreadState old_state_;
  DISALLOW_COPY_AND_ASSIGN(ApiObjectAccessScope);
};

class ThreadSuspensionScope {
 public:
  explicit ThreadSuspensionScope(VMThread* self);
 ~ThreadSuspensionScope();

 private:
  VMThread* const self_;
  DISALLOW_COPY_AND_ASSIGN(ThreadSuspensionScope);
};


class PauseScope final {
  public:
    explicit PauseScope(VMThread* self);
    ~PauseScope();
  private:
    VMThreadList* thread_list_;
    ThreadStateChangeScope tsc_;
    DISALLOW_COPY_AND_ASSIGN(PauseScope);
};


}  // namespace internal
}  // namespace v8
#endif  // THREAD_THREAD_STATE_CHANGE_H_
