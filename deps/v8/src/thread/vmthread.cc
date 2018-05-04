/**
 * {{{ Copyright (C) 2015 The YunOS Project. All rights reserved. }}}
 */


#include "src/deoptimizer.h"
#include "src/ic/stub-cache.h"
#include "src/isolate-inl.h"
#include "src/objects.h"
#include "src/thread/vmthread-inl.h"
#include "src/thread/sync/mutex-inl.h"
#include "src/regexp/regexp-stack.h"
#include "src/regexp/jsregexp.h"
#include "src/heap/store-buffer.h"
#include "src/lookup-cache.h"
#include "src/ast/context-slot-cache.h"
#include "src/handles.h"
#include "src/simulator.h"
#include "src/debug/debug.h"
#include "src/thread/thread-state-change-inl.h"
#include "src/thread/sync/monitor-inl.h"
#include "src/thread/vmthread-inl.h"
#include "src/snapshot/snapshot.h"


namespace v8 {
namespace internal {

static const uint32_t kThreadSuspendTimeoutMs = 30 * 1000;
static const uint32_t kThreadSuspendInitialSleepUs = 0;
static const uint32_t kThreadSuspendMaxYieldUs = 3000;
static const uint32_t kThreadSuspendMaxSleepUs = 5000;

VMThread::VMThread() :
  suspend_count_(0),
  thread_id_(VMThreadList::kInvalidThreadId),
  nesting_in_suspend_all_(0),
  js_worker_(NULL),
  wait_next_vmthread_(NULL),
  wait_monitor_(NULL),
  checkpoint_function_(NULL),
  worker_callback_() {
  flags_and_state_.as_int = 0;
  pre_thread_flags_ = PreThreadFlags(NO_FLAGS);
  for (uint32_t i = 0; i < kMaxSuspendBarriers; i++) {
    active_suspend_barriers_[i] = NULL;
  }
  memset(&held_mutexes_[0], 0, sizeof(held_mutexes_));
  wait_mutex_ = new sync::Mutex("a thread wait mutex");
  wait_cond_ = new sync::ConditionVariable("a thread wait condition variable", *wait_mutex_);
  STATIC_ASSERT(kSmiTagSize == 1);
}


VMThread::~VMThread() {}


void VMThread::TearDown() {
  set_state(kTerminated);

  delete js_worker_;
  js_worker_ = NULL;

  delete wait_mutex_;
  wait_mutex_ = NULL;
  delete wait_cond_;
  wait_cond_ = NULL;
}

void VMThread::Start() {
  DCHECK(!is_main());
  VMThreadList* thread_list = this->thread_list();
  VMThread* self = VMThread::Current();
  if (!thread_list->StartThreadBirth(self)) {
    CHECK(false);
    return;
  }
  thread_list->only_main_thread_exist_ = false;
  self->ClearPreThreadFlag(SINGLE_THREAD_MODE);
  DCHECK(js_worker_ == NULL);
  js_worker_ = new JSWorker(this);
  DCHECK(js_worker_ != NULL);
  CHECK(js_worker_->Start());
}


void VMThread::Run() {
  js_worker_->Detach();
  CHECK(state() == kRunnable);

  DCHECK(!is_main());
  if (worker_callback_.callback != nullptr) {
    TransitionFromRunnableToSuspended(kNative);
    v8::Isolate* v8_isolate = reinterpret_cast<v8::Isolate*>(isolate());
    worker_callback_.callback(v8_isolate, worker_callback_.data);
    TransitionFromSuspendedToRunnable();
  }
}


void VMThread::JSWorker::Run() {
  Isolate* isolate = vm_thread_->isolate();
  isolate->Enter();
  // CHECK(Snapshot::Initialize(isolate));
  isolate->Init(NULL);
  VMThreadList* thread_list = vm_thread_->thread_list();
  thread_list->EndThreadBirth(vm_thread_);
  vm_thread_->Run();
  isolate->Exit();
  isolate->TearDown();
}


bool VMThread::Initialize() {
  VMThreadList* thread_list = this->thread_list();
  // At that time heap has not been initialized, so the hole is not available.
  // After deserialization is done, main-thread will clear these exceptions at
  // the end of "Isolate::Init"
  thread_id_ = thread_list->AllocThreadId();
  thread_list->Register(this);
  TransitionFromSuspendedToRunnable();
  return true;
}

bool VMThread::AllocateThreadId() {
  // At that time heap has not been initialized, so the hole is not available.
  // After deserialization is done, main-thread will clear these exceptions at
  // the end of "Isolate::Init"
  thread_id_ = thread_list()->AllocThreadId();
  return true;
}

bool VMThread::RegisterAndToRunnable() {
  thread_list()->Register(this);
  TransitionFromSuspendedToRunnable();
  return true;
}

void VMThread::Destroy() {
  TransitionFromRunnableToSuspended(kWait);

  thread_list()->Unregister(this);
  TearDown();
}

bool VMThread::ModifySuspendCountInternal(int32_t delta,
                                  sync::Barrier* suspend_barrier) {
  if (V8_UNLIKELY(delta < 0 && suspend_count_ <= 0)) {
    V8_Fatal(__FILE__, __LINE__, "invalid suspend count.");
    return false;
  }
  uint32_t flags = kSuspendRequest;
  if (delta > 0 && suspend_barrier != NULL) {
    uint32_t available_barrier = kMaxSuspendBarriers;
    for (uint32_t i = 0; i < kMaxSuspendBarriers; i++) {
      if (active_suspend_barriers_[i] == NULL) {
        available_barrier = i;
        break;
      }
    }
    if (available_barrier == kMaxSuspendBarriers) {
      return false;
    }
    active_suspend_barriers_[available_barrier] = suspend_barrier;
    flags |= kActiveSuspendBarrier;
  }
  suspend_count_ += delta;
  if (suspend_count_ == 0) {
    AtomicClearFlag(kSuspendRequest);
    // JSMT: TODO  stack guard is slow
    stack_guard()->ClearInterrupt(StackGuard::SUSPEND_AND_CHECKPOINT);
  } else {
    flags_and_state_.as_atomic_int.FetchAndOrSequentiallyConsistent(flags);
    // JSMT: TODO  stack guard is slow
    stack_guard()->RequestInterrupt(StackGuard::SUSPEND_AND_CHECKPOINT);
  }
  // Clear SuspendAll Interrupt, see StackGuard::HandleInterrupts()
  return true;
}


void VMThread::ClearSuspendBarrier(sync::Barrier* target) {
  CHECK(HasFlag(kActiveSuspendBarrier));
  bool clear_flag = true;
  for (uint32_t i = 0; i < kMaxSuspendBarriers; i++) {
    sync::Barrier* ptr = active_suspend_barriers_[i];
    if (ptr == target) {
      active_suspend_barriers_[i] = NULL;
      break;
    } else if (ptr != NULL) {
      clear_flag = false;
    }
  }
  if (clear_flag) {
    AtomicClearFlag(kActiveSuspendBarrier);
  }
}


void VMThread::PassActiveSuspendBarriers() {
  sync::Barrier* pass_barriers[kMaxSuspendBarriers];
  {
    VMThreadList* threads = thread_list();
    sync::MutexLock mu(this, threads->thread_suspend_count_lock_);
    if (!HasFlag(kActiveSuspendBarrier)) return;
    for (uint32_t i = 0; i < kMaxSuspendBarriers; ++i) {
      pass_barriers[i] = active_suspend_barriers_[i];
      active_suspend_barriers_[i] = NULL;
    }
    AtomicClearFlag(kActiveSuspendBarrier);
  }

  for (uint32_t i = 0; i < kMaxSuspendBarriers; ++i) {
    sync::Barrier* pending_thread_barrier = pass_barriers[i];
    if (pending_thread_barrier != NULL) {
      pending_thread_barrier->Pass();
    }
  }
}

bool VMThread::RequestCheckpoint(Closure* function) {
  union FlagsAndState old_flags_and_state;
  union FlagsAndState new_flags_and_state;
  old_flags_and_state.as_int = flags_and_state_.as_int;
  if (old_flags_and_state.i.as_state != kRunnable) {
    return false;
  }
  new_flags_and_state.as_int = old_flags_and_state.as_int | kCheckpointRequest;
  bool success =
      flags_and_state_.as_atomic_int.CompareExchangeStrongSequentiallyConsistent(
                                         old_flags_and_state.as_int,
                                         new_flags_and_state.as_int);
  if (success) {
    if (checkpoint_function_ == NULL) {
      checkpoint_function_ = function;
    } else {
      checkpoint_overflow_.Add(function);
    }
    stack_guard()->RequestInterrupt(StackGuard::SUSPEND_AND_CHECKPOINT);
  }
  return success;
}

void VMThread::RunCheckpointFunction() {
  bool done = false;
  VMThreadList* threads = thread_list();
  DCHECK(this == VMThread::Current());

  do {
    Closure* checkpoint = NULL;
    {
      sync::MutexLock mu(this, threads->thread_suspend_count_lock_);
      if (checkpoint_function_ != NULL) {
        checkpoint = checkpoint_function_;
        if (!checkpoint_overflow_.is_empty()) {
          checkpoint_function_ = checkpoint_overflow_.RemoveLast();
        } else {
          checkpoint_function_ = NULL;
          AtomicClearFlag(kCheckpointRequest);
          done = true;
        }
      } else {
        V8_Fatal(__FILE__, __LINE__, "Checkpoint flag set without pending checkpoint");
      }
    }
    checkpoint->Run(this);
  } while (!done);
}

void VMThread::CheckSuspend() {
  while(true) {
    if (AtomicHasFlag(kCheckpointRequest)) {
      RunCheckpointFunction();
    } else if (AtomicHasFlag(kSuspendRequest)){
      ThreadSuspensionScope tss(this);
    } else {
      break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////

VMThreadList::VMThreadList(WorkerRuntime* worker_runtime) :
  worker_runtime_(worker_runtime),
  main_thread_(worker_runtime->main_isolate()->vmthread()),
  zone_(worker_runtime->main_isolate()->allocator(), "thread list zone"),
  mutator_lock_("mutator lock", sync::kMutatorLock),
  threads_(256),
  threads_lock_("threads lock"),
  thread_suspend_count_lock_("thread suspend count lock"),
  suspend_all_lock_("suspend all lock"),
  resume_cond_("resume cond", thread_suspend_count_lock_),
  borning_count_(0),
  suspend_all_count_(0),
  allocated_ids_(kMaxThreadId, &zone_),
  allocated_ids_lock_("alloc ids lock"),
  shutdown_lock_("shutdown lock"),
  thread_exit_cond_("thread exit cond", threads_lock_),
  in_suspend_all_scope_(false),
  only_main_thread_exist_(true),
#ifdef __LP64__
  monitor_pool_(sync::MonitorPool::Create()),
#endif
  monitor_list_(new sync::MonitorList(this)) {
  allocated_ids_.Clear();
}


VMThreadList::~VMThreadList() {
  delete monitor_list_;
#ifdef __LP64__
  delete monitor_pool_;
#endif
}

VMThread* VMThreadList::FindThreadByThreadIdUnSafe(uint32_t thread_id) {
  for (int i = 0; i < threads_.length(); i++) {
    if (threads_[i]->thread_id() == thread_id) return threads_[i];
  }
  return NULL;
}


void VMThreadList::RequestInterrupts(VMThread* self,
                                     StackGuard::InterruptFlag flag) {
  sync::MutexLock mu(self, threads_lock_);
  for (int i = 0; i < threads_.length(); ++i) {
    threads_[i]->stack_guard()->RequestInterrupt(flag);
  }
}


void VMThreadList::ClearInterrupts(VMThread* self,
                                   StackGuard::InterruptFlag flag) {
  sync::MutexLock mu(self, threads_lock_);
  for (int i = 0; i < threads_.length(); ++i) {
    threads_[i]->stack_guard()->ClearInterrupt(flag);
  }
}


void VMThreadList::StopAllOtherThreads() {
  VMThread* self = VMThread::Current();
  CHECK(self == main_thread_);
  {
    sync::MutexLock mu(self, shutdown_lock_);
    worker_runtime_->set_runtime_state(WorkerRuntimeState::kSuspend);
  }
  ThreadStateChangeScope tsc(self, kSuspended);
  WaitForOtherThreadsExit(self, false);
  only_main_thread_exist_ = true;
  self->SetPreThreadFlag(VMThread::SINGLE_THREAD_MODE);
}


void VMThreadList::JoinAllJsThreads() {
  VMThread* self = VMThread::Current();
  CHECK(self == main_thread_);
  ThreadStateChangeScope tsc(self, kSuspended);
  {
    sync::MutexLock mu(self, shutdown_lock_);
    worker_runtime_->set_runtime_state(WorkerRuntimeState::kSuspend);
  }
  WaitForOtherThreadsExit(self, true);
  worker_runtime_->set_runtime_state(WorkerRuntimeState::kInitialized);
}


void VMThreadList::WaitForOtherThreadsExit(VMThread* self, bool skip_join_all) {
  bool done = true;
  do {
    done = true;
    sync::MutexLock mu(self, threads_lock_);
    if (borning_count_locked(self) != 0) {
      done = false;
    } else {
      for (int i = 0; i < threads_.length(); ++i) {
        VMThread* thread = threads_[i];
        if (thread == self) {
          continue;
        }
        if (skip_join_all && thread->skip_join_all()) {
          continue;
        }
        done = false;
        break;
      }
    }
    if (!done) thread_exit_cond_.Wait(self);
  } while (!done);
}


uint32_t VMThreadList::borning_count_locked(VMThread* self) {
  sync::MutexLock mu(self, shutdown_lock_);
  return borning_count_;
}


bool VMThreadList::StartThreadBirth(VMThread* self) {
  sync::MutexLock mu(self, shutdown_lock_);
  if (stop_birth()) return false;
  borning_count_++;
  return true;
}


void VMThreadList::EndThreadBirth(VMThread* self) {
  sync::MutexLock mu(self, shutdown_lock_);
  DCHECK(borning_count_ > 0);
  borning_count_--;
}


void VMThreadList::Register(VMThread* self) {
  sync::MutexLock mu(self, threads_lock_);
  sync::MutexLock mu1(self, thread_suspend_count_lock_);
  for (uint32_t i = suspend_all_count_; i > 0; i--) {
    self->ModifySuspendCount(self, +1, NULL);
  }
  threads_.Add(self);
}


void VMThreadList::Unregister(VMThread* self) {
  CHECK(self->state() != kRunnable);
  while (true) {
    // unlock may after thread_list destroy the mutex.
    // because we are not in threads_
    sync::MutexLock mu(self, threads_lock_);
    if (!threads_.Contains(self)) {
      // JSMT TODO Log...
      break;
    } else {
      sync::MutexLock mu1(self, thread_suspend_count_lock_);
      if (self->suspend_count_ == 0) {
        threads_.RemoveElement(self);
        break;
      }
    }
  }
  ReleaseThreadId(self->thread_id());
  sync::MutexLock mu(NULL, threads_lock_);
  thread_exit_cond_.Broadcast(NULL);
}


uint32_t VMThreadList::AllocThreadId() {
  sync::MutexLock mu(NULL, allocated_ids_lock_);
  for (int i = 0; i < allocated_ids_.length(); ++i) {
    if (!allocated_ids_.Contains(i)) {
      allocated_ids_.Add(i);
      // Note: id starts from 1 instead of 0.
      return i + 1;
    }
  }
  UNREACHABLE();
  return kInvalidThreadId;
}


void VMThreadList::ReleaseThreadId(uint32_t id) {
  sync::MutexLock mu(NULL, allocated_ids_lock_);
  id--;
  DCHECK(allocated_ids_.Contains(id));
  allocated_ids_.Remove(id);
}


void VMThreadList::SuspendAll() {
  VMThread* self = VMThread::Current();
  // Isolate is exit.
  if (self == nullptr) return;
  if (self->IsInSuspendAll()) {
    CHECK(self->nesting_in_suspend_all_ >= 1);
    self->nesting_in_suspend_all_++;
    return;
  }
  suspend_all_lock_.ExclusiveLock(self);
  self->nesting_in_suspend_all_++;
  SuspendAllInternal(self);
#if HAVE_TIMED_RWLOCK
  while (true) {
    if (mutator_lock_.ExclusiveLockWithTimeout(self,
        base::TimeDelta::FromMilliseconds(kThreadSuspendTimeoutMs))) {
      break;
    } else {
      V8_Fatal(__FILE__, __LINE__, "Thread suspend timeout after 30s.");
    }
  }
#else
  mutator_lock_.ExclusiveLock(self);
#endif
  in_suspend_all_scope_ = true;
}

void VMThreadList::SuspendAllInternal(VMThread* self) {
  mutator_lock_.AssertNotExclusiveHeld(self);
  threads_lock_.AssertNotHeld(self);
  thread_suspend_count_lock_.AssertNotHeld(self);

  sync::Barrier pending_thread_barrier;
  {
    sync::MutexLock mu(self, threads_lock_);
    ++suspend_all_count_;
    if (threads_.length() <= 1) {
      DCHECK(self == main_thread_);
      return;
    }
    sync::MutexLock mu1(self, thread_suspend_count_lock_);
    pending_thread_barrier.Init(threads_.length() - 1);
    for (const auto& thread : threads_) {
      if (thread == self) {
        continue;
      }

      thread->ModifySuspendCount(self, +1, &pending_thread_barrier);
      if (thread->IsSuspended()) {
        // Only clear the counter for the current thread.
        thread->ClearSuspendBarrier(&pending_thread_barrier);
        pending_thread_barrier.PassNoWeak();
      }
    }
  }
  pending_thread_barrier.Wait();
}


void VMThreadList::ResumeAll() {
  VMThread* self = VMThread::Current();
  if (self == nullptr) return;
  self->nesting_in_suspend_all_--;
  in_suspend_all_scope_ = false;
  if (self->nesting_in_suspend_all_ != 0) {
    CHECK(self->nesting_in_suspend_all_ > 0);
    return;
  }
  mutator_lock_.ExclusiveUnlock(self);
  {
    sync::MutexLock mu(self, threads_lock_);
    --suspend_all_count_;
    sync::MutexLock mu1(self, thread_suspend_count_lock_);
    for (const auto& thread : threads_) {
      if (thread == self) {
        continue;
      }
      thread->ModifySuspendCount(self, -1, NULL);
    }
    resume_cond_.Broadcast(self);
  }
  suspend_all_lock_.ExclusiveUnlock(self);
}


void VMThreadList::Resume(VMThread* thread) {
  VMThread* self = VMThread::Current();
  {
    sync::MutexLock mu(self, threads_lock_);
    sync::MutexLock mu1(self, thread_suspend_count_lock_);
    if (!threads_.Contains(thread)) {
      // JSMT TODO Log...
      return;
    }
    thread->ModifySuspendCount(self, -1, NULL);
    resume_cond_.Broadcast(self);
  }
}


VMThread* VMThreadList::SuspendThreadByThreadId(
  uint32_t thread_id, bool* time_out) {
  VMThread* self = VMThread::Current();
  *time_out = false;

  base::Time start_time = base::Time::Now();
  base::Time end_time = start_time +
      base::TimeDelta::FromMilliseconds(kThreadSuspendTimeoutMs);
  uint32_t sleep_us = kThreadSuspendInitialSleepUs;
  VMThread* suspended_thread = NULL;

  while(true) {
    {
      ThreadStateChangeScope tcs(self, kRunnable);
      sync::MutexLock mu(self, threads_lock_);
      VMThread* thread = NULL;
      for (const auto& it : threads_) {
        if (it->thread_id() == thread_id) {
          thread = it;
        }
      }
      if (thread == NULL) {
        return NULL;
      }
      {
        sync::MutexLock mu1(self, thread_suspend_count_lock_);
        if (suspended_thread == NULL) {
          if (self->suspend_count_ > 0) {
            // We hold the suspend count lock but another thread
            // is trying to suspend us.
            continue;
          }
          thread->ModifySuspendCount(self, +1, NULL);
          suspended_thread = thread;
        } else {
          DCHECK_EQ(suspended_thread, thread);
          CHECK_GT(thread->suspend_count_, 0);
        }
        if (thread->IsSuspended()) {
          return thread;
        }
        base::Time now = base::Time::Now();
        uint64_t total_delay = (now - start_time).InMicroseconds();
        if (now >= end_time) {
          if (suspended_thread != NULL) {
            DCHECK_EQ(suspended_thread, thread);
            thread->ModifySuspendCount(self, -1, NULL);
          }
          *time_out = true;
          return NULL;
        } else if (sleep_us == 0 && total_delay > kThreadSuspendMaxYieldUs) {
          // We have spun for kThreadSuspendMaxYieldUs time,
          //  switch to sleeps to prevent excessive CPU usage.
          sleep_us = kThreadSuspendMaxYieldUs / 2;
        }
      }
    }
    if (sleep_us == 0) {
      sched_yield();
    } else {
      base::OS::Sleep(base::TimeDelta::FromMicroseconds(sleep_us));
    }
    sleep_us = std::min(sleep_us * 2, kThreadSuspendMaxSleepUs);
  }
  return NULL;
}

uint32_t VMThreadList::RunCheckpoint(Closure* function) {
  VMThread* self = VMThread::Current();
  mutator_lock_.AssertNotExclusiveHeld(self);
  threads_lock_.AssertNotHeld(self);
  thread_suspend_count_lock_.AssertNotHeld(self);

  Zone zone(self->isolate()->allocator(), ZONE_NAME);
  ZoneQueue<VMThread*> suspended_count_modified_threads(&zone);
  uint32_t count = 0;
  {
    sync::MutexLock mu(self, threads_lock_);
    sync::MutexLock mu1(self, thread_suspend_count_lock_);
    count = threads_.length();
    for (const auto& thread : threads_) {
      if (thread == self) continue;
      while (true) {
        if (thread->RequestCheckpoint(function)) {
          break;
        } else {
          // Spurious fail, try again.
          if (thread->state() == kRunnable) continue;
          thread->ModifySuspendCount(self, +1, NULL);
          suspended_count_modified_threads.push(thread);
          break;
        }
      }
    }
  }
  function->Run(self);

  while (!suspended_count_modified_threads.empty()) {
    VMThread* thread = suspended_count_modified_threads.front();
    suspended_count_modified_threads.pop();
    if (!thread->IsSuspended()) {
      base::Time start_time = base::Time::Now();
      do {
        if (kThreadSuspendInitialSleepUs == 0) {
          sched_yield();
        } else {
          base::OS::Sleep(
            base::TimeDelta::FromMicroseconds(kThreadSuspendInitialSleepUs));
        }
      } while (!thread->IsSuspended());
      base::Time now = base::Time::Now();
      uint64_t total_delay = (now - start_time).InMicroseconds();
      if (total_delay > 1000) {
        PrintF("Long wait of RunCheckpoint on thread %d", thread->thread_id());
      }
    }
    function->Run(thread);
    {
      sync::MutexLock mu1(self, thread_suspend_count_lock_);
      thread->ModifySuspendCount(self, -1, NULL);
    }
  }

  {
    sync::MutexLock mu1(self, thread_suspend_count_lock_);
    resume_cond_.Broadcast(self);
  }
  return count;
}

void VMThreadList::PrintAllThreadStates() {
  for (const auto& thread : threads_) {
    PrintF("%d : %d\n", thread->thread_id(), thread->state());
  }
}

ThreadIterator::ThreadIterator(VMThreadList* thread_list, VMThread* self) :
  cur_(-1),
  lock_(false),
  current_(NULL),
  thread_list_(thread_list),
  self_(self) {
    if (!self) self = self_ = VMThread::Current();
    // The Iterator maybe used in PauseScope
    if (!thread_list->threads_lock_.IsExclusiveHeld(self)) {
      thread_list->threads_lock_.ExclusiveLock(self);
      lock_ = true;
    }
    DCHECK(lock_ || self->IsInSuspendAll());
    count_ = thread_list->threads_.length();
    Advance();
  }

void ThreadIterator::Advance() {
  cur_++;
  if (Done()) {
    if (lock_) {
      lock_ = false;
      thread_list_->threads_lock_.ExclusiveUnlock(self_);
    }
    current_ = NULL;
    return;
  }
  current_ = thread_list_->threads_[cur_];
}

ThreadIterator::~ThreadIterator() {
  if (!Done() && lock_) {
    thread_list_->threads_lock_.ExclusiveUnlock(self_);
  }
}

}  // namespace internal
}  // namespace v8
