/**
 * {{{ Copyright (C) 2015 The YunOS Project. All rights reserved. }}}
 */

#ifndef THREAD_VMTHREAD_H_
#define THREAD_VMTHREAD_H_

#include <queue>
#include "src/base/platform/platform.h"
#include "src/bit-vector.h"
#include "src/execution.h"
#include "src/globals.h"
#include "src/list.h"
#include "src/thread/sync/monitor.h"
#include "src/thread/thread-state-change.h"
#include "src/thread/sync/barrier.h"
#include "src/thread/worker-runtime.h"

namespace v8 {
namespace internal {

class VMThreadList;
class Isolate;
class WorkerRuntime;

class Closure {
 public:
  virtual ~Closure() { }
  virtual void Run(VMThread* self) = 0;
};

class VMThread {
 public:
  static const int kDefaultStackSize = 2 * MB;
  static const uint32_t kMaxSuspendBarriers = 3;

  enum PreThreadFlag {
    NO_FLAGS = 0u,
    IS_MAIN = 1u << 0,
    SKIP_JOIN_ALL = 1u << 1,
    SINGLE_THREAD_MODE = 1u << 2,
    IN_SHARED_CODEGEN = 1u << 3,
    FAST_API_CALL = 1u << 5,
  };

  typedef base::Flags<PreThreadFlag, uintptr_t> PreThreadFlags;

  static VMThread* Current();

  VMThread();
  ~VMThread();
  void Destroy();
  bool Initialize();
  bool AllocateThreadId();
  bool RegisterAndToRunnable();
  void TearDown();
  void Start();
  // JSMT TODO: BUGID:8944968. Thread join is unsafe.
  void Join() { if(js_worker_ != NULL) js_worker_->Join(); }

  uint32_t thread_id() const { return thread_id_; }
  Isolate* isolate();

  sync::BaseMutex* held_mutex(sync::LockLevel level) const {
     return held_mutexes_[level];
  }
  void set_held_mutex(sync::LockLevel level, sync::BaseMutex* mutex) {
    held_mutexes_[level] = mutex;
  }

  void ModifySuspendCount(VMThread* self, int32_t delta, sync::Barrier* suspend_barrier);
  void ClearSuspendBarrier(sync::Barrier* target);
  void PassActiveSuspendBarriers();

  void TransitionFromSuspendedToRunnable();
  void TransitionFromRunnableToSuspended(VMThreadState new_state);

  void set_wait_monitor(sync::Monitor* mon) { wait_monitor_ = mon; }
  sync::Monitor* wait_monitor() const { return wait_monitor_; }
  void set_wait_next_vmthread(VMThread* next) { wait_next_vmthread_ = next; }
  VMThread* wait_next_vmthread() const { return wait_next_vmthread_; }
  sync::ConditionVariable* wait_condition_variable() const { return wait_cond_; }
  sync::Mutex* wait_mutex() const { return wait_mutex_; }

  void AssertCurrentThread();
  void AssertMutatorRunnable();
  bool IsCurrentThread();
  bool IsThreadSafe();
  bool IsInSuspendAll() const { return nesting_in_suspend_all_ != 0; }
  bool IsJSThread();
  bool RequestCheckpoint(Closure* function);
  void RunCheckpointFunction();
  bool IsSuspended() {
    return ((flags_and_state_.i.as_state != kRunnable) &&
            (flags_and_state_.i.as_flags & kSuspendRequest) != 0);
  }

  VMThreadState state() const {
    return static_cast<VMThreadState>(flags_and_state_.i.as_state);
  }

  void CheckSuspend();

  void set_state(VMThreadState state) {
    union FlagsAndState old_flags_and_state;
    old_flags_and_state.as_int = flags_and_state_.as_int;
    CHECK_NE(old_flags_and_state.i.as_state, kRunnable);
    flags_and_state_.i.as_state = state;
  }

  void SetPreThreadFlag(PreThreadFlag flag) { pre_thread_flags_ |= flag; }
  void ClearPreThreadFlag(PreThreadFlag flag) {
    pre_thread_flags_ &= ~PreThreadFlags(flag);
  }
  bool IsPreThreadFlagSet(PreThreadFlag flag) const {
    return pre_thread_flags_ & flag;
  }

  bool is_main() const { return IsPreThreadFlagSet(IS_MAIN); }
  bool skip_join_all() const { return IsPreThreadFlagSet(SKIP_JOIN_ALL); }
  bool in_shared_codegen() const { return IsPreThreadFlagSet(IN_SHARED_CODEGEN); }
  bool single_thread_mode() const {
    return IsPreThreadFlagSet(SINGLE_THREAD_MODE);
  }
  void set_skip_join_all(bool skip_join_all) {
    if (skip_join_all) SetPreThreadFlag(SKIP_JOIN_ALL);
    else ClearPreThreadFlag(SKIP_JOIN_ALL);
  }
  VMThreadList* thread_list() const { return thread_list_; }
  StackGuard* stack_guard();
  void set_worker_callback(v8::WorkerStartCallback worker_callback) {
    worker_callback_ = worker_callback;
  }
  void MarkMainThreadFlags() {
    pre_thread_flags_ |= static_cast<PreThreadFlag>(IS_MAIN|SINGLE_THREAD_MODE);
  }

 private:
  class JSWorker : public base::Thread {
   public:
    explicit JSWorker(VMThread* thread)
      : base::Thread(base::Thread::Options("js thread", kDefaultStackSize))
      , vm_thread_(thread) {}
    void Run() override;

   private:
    VMThread* vm_thread_;
  };

  union FlagsAndState {
    FlagsAndState() {}
    sync::AtomicInteger as_atomic_int;
    uint32_t as_int;
    struct FlagsOrState {
      volatile uint16_t as_flags;
      volatile uint16_t as_state;
    };
    FlagsOrState i;
   private:
    DISALLOW_COPY_AND_ASSIGN(FlagsAndState);
  };

  void Run();
  bool ModifySuspendCountInternal(int32_t delta, sync::Barrier* suspend_barrier);
  bool HasFlag(VMThreadFlag flag) {
    return (flags_and_state_.i.as_flags & flag) != 0;
  }
  bool AtomicHasFlag(VMThreadFlag flag) {
    uint32_t value = flags_and_state_.as_atomic_int.LoadAcquire();
    FlagsAndState flags_and_state;
    flags_and_state.as_int = value;
    return ((flags_and_state.i.as_flags & flag) != 0);
  }
  void AtomicSetFlag(VMThreadFlag flag) {
      flags_and_state_.as_atomic_int.FetchAndOrSequentiallyConsistent(flag);
  }
  void AtomicClearFlag(VMThreadFlag flag) {
    flags_and_state_.as_atomic_int.FetchAndAndSequentiallyConsistent(-1 ^ flag);
  }

  int32_t suspend_count_;
  uint32_t thread_id_;
  int32_t nesting_in_suspend_all_;
  FlagsAndState flags_and_state_;
  PreThreadFlags pre_thread_flags_;

  VMThreadList* thread_list_;
  sync::Barrier* active_suspend_barriers_[kMaxSuspendBarriers];
  sync::BaseMutex* held_mutexes_[sync::kLockLevelCount];
  JSWorker* js_worker_;
  sync::Mutex* wait_mutex_;
  sync::ConditionVariable* wait_cond_;
  VMThread* wait_next_vmthread_;
  sync::Monitor* wait_monitor_;
  Closure* checkpoint_function_;
  List<Closure*> checkpoint_overflow_;
  v8::WorkerStartCallback worker_callback_;


  friend class VMThreadList;
  friend class Isolate;
};

#undef THREAD_LOCAL_TOP_ACCESSOR

class SharedCodegenScope {
 public:
  SharedCodegenScope(VMThread* self, bool set_flag = true)
    : self_(self), flag_is_set_(false) {
    if (set_flag && !self->in_shared_codegen()) {
      self->SetPreThreadFlag(VMThread::IN_SHARED_CODEGEN);
      flag_is_set_ = true;
    }
  }

  ~SharedCodegenScope() {
    if (flag_is_set_) {
      self_->ClearPreThreadFlag(VMThread::IN_SHARED_CODEGEN);
    }
  }

  void Release() {
    if (flag_is_set_) {
      self_->ClearPreThreadFlag(VMThread::IN_SHARED_CODEGEN);
      flag_is_set_ = false;
    }
  }

 private:
  VMThread* self_;
  bool flag_is_set_;
  DISALLOW_COPY_AND_ASSIGN(SharedCodegenScope);
};

class UnSharedCodegenScope {
 public:
  UnSharedCodegenScope(VMThread* self, bool clear_flag = true)
    : self_(self), flag_is_clear_(false) {
    if (clear_flag && self->in_shared_codegen()) {
      self->ClearPreThreadFlag(VMThread::IN_SHARED_CODEGEN);
      flag_is_clear_ = true;
    }
  }

  ~UnSharedCodegenScope() {
    if (flag_is_clear_) {
      self_->SetPreThreadFlag(VMThread::IN_SHARED_CODEGEN);
    }
  }

 private:
  VMThread* self_;
  bool flag_is_clear_;
  DISALLOW_COPY_AND_ASSIGN(UnSharedCodegenScope);
};

class FastApiCallScope {
 public:
  FastApiCallScope(VMThread* self, bool set_flag)
    : self_(self) {
    DCHECK(!self->IsPreThreadFlagSet(VMThread::FAST_API_CALL));
    if (set_flag)
      self->SetPreThreadFlag(VMThread::FAST_API_CALL);
  }

  ~FastApiCallScope() {
    self_->ClearPreThreadFlag(VMThread::FAST_API_CALL);
  }

 private:
  VMThread* self_;
  DISALLOW_COPY_AND_ASSIGN(FastApiCallScope);
};


class VMThreadList {
 public:
  static const uint32_t kInvalidThreadId = 0xFFFFFFFF;
  static const uint32_t kMaxThreadId = (1 << sync::LockWord::kThinLockOwnerSize) - 1;

  explicit VMThreadList(WorkerRuntime* worker_runtime);
  ~VMThreadList();

  void PrintAllThreadStates();

  /*
   * if the isolate has been shutdown or is susped
   * do not allow new thread birth
   * */
  bool stop_birth() {
    return worker_runtime_->runtime_state() == WorkerRuntimeState::kShuttingDown ||
              worker_runtime_->runtime_state() == WorkerRuntimeState::kSuspend;
  }
  void JoinAllJsThreads();

  size_t GetThreadsCount() const { return threads_.length(); }

  void SuspendAll();
  void ResumeAll();
  void Resume(VMThread* thread);
  VMThread* FindThreadByThreadIdUnSafe(uint32_t thread_id);
  VMThread* SuspendThreadByThreadId(uint32_t thread_id, bool* time_out);

  /*Daemo thread for background compiler*/
  bool only_main_thread_exist() const { return only_main_thread_exist_; }

  void WaitForOtherThreadsExit(VMThread* self, bool skip_join_all);
  void Register(VMThread* self);
  void Unregister(VMThread* self);
  uint32_t AllocThreadId();
  void ReleaseThreadId(uint32_t id);
  bool StartThreadBirth(VMThread* self);
  void EndThreadBirth(VMThread* self);

  void StopAllOtherThreads();
  void ClearInterrupts(VMThread* self, StackGuard::InterruptFlag flag);
  void RequestInterrupts(VMThread* self, StackGuard::InterruptFlag flag);
  uint32_t RunCheckpoint(Closure* function);

#ifdef __LP64__
  sync::MonitorPool* monitor_pool() const { return monitor_pool_; }
#endif
  sync::MonitorList* monitor_list() const { return monitor_list_; }

  void AssertSharedHeldMutatorLock(VMThread* self) {
    mutator_lock_.AssertSharedHeld(self);
  }
  WorkerRuntime* worker_runtime() const { return worker_runtime_; }

 private:
  uint32_t borning_count_locked(VMThread* thread);
  void SuspendAllInternal(VMThread* self);

  WorkerRuntime* worker_runtime_;
  VMThread* main_thread_;
  Zone zone_;

  sync::ReaderWriterMutex mutator_lock_;
  List<VMThread*> threads_;
  sync::Mutex threads_lock_;
  sync::Mutex thread_suspend_count_lock_;
  sync::Mutex suspend_all_lock_;
  sync::ConditionVariable resume_cond_;

  uint32_t borning_count_;
  uint32_t suspend_all_count_;
  BitVector allocated_ids_;
  sync::Mutex allocated_ids_lock_;

  sync::Mutex shutdown_lock_;
  sync::ConditionVariable thread_exit_cond_;

  bool in_suspend_all_scope_;
  bool only_main_thread_exist_;

#ifdef __LP64__
  sync::MonitorPool* monitor_pool_;
#endif
  sync::MonitorList* monitor_list_;

  friend class VMThread;
  friend class ThreadIterator;
  friend class WorkerRuntime;
  friend class sync::Monitor;
  friend class sync::MonitorList;
  friend class Isolate;
};


}  // namespace internal
}  // namespace v8
#endif  // THREAD_VMTHREAD_H_
