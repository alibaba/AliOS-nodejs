/**
 * {{{ Copyright (C) 2017 The YunOS Project. All rights reserved. }}}
 */
#include "src/thread/worker-runtime.h"
#include "src/thread/vmthread.h"
#include "src/isolate.h"
#include "src/v8.h"
#include "src/snapshot/snapshot.h"
#include "src/objects-inl.h"
#include "src/interpreter/interpreter.h"
#include "src/simulator.h"

namespace v8 {
namespace internal {

base::Thread::LocalStorageKey WorkerRuntime::runtime_key_;

void WorkerRuntime::InitializeOncePerProcess() {
  WorkerRuntime::runtime_key_ = base::Thread::CreateThreadLocalKey();
#if DEBUG
  if (base::Thread::GetThreadLocal(WorkerRuntime::runtime_key_) != nullptr) {
    FATAL("Newly-created thread TLS slot is not nullptr");
  }
#endif
}

bool WorkerRuntime::IsShuttingDown() {
  WorkerRuntime* runtime = WorkerRuntime::Current();
  return (runtime == nullptr ||
          runtime->runtime_state() == WorkerRuntimeState::kShuttingDown);
}


Isolate* WorkerRuntime::CreateNewWorker(v8::WorkerStartCallback worker_callback) {
  DCHECK(WorkerRuntime::Current() == this);
  Isolate* isolate = new Isolate(false, this);
  // isolate->set_snapshot_blob(Snapshot::DefaultSnapshotBlob());
  VMThread* vm_thread = isolate->vmthread();
  vm_thread->set_worker_callback(worker_callback);
  return isolate;
}

WorkerRuntime::WorkerRuntime(Isolate* main_isolate) :
              main_isolate_(main_isolate),
              runtime_state_(WorkerRuntimeState::kUnInitialize),
              shared_external_string_table_(this),
              tracer_(nullptr),
              total_shared_gc_time_ms_(0.0),
              shared_space_allocation_counter_at_last_gc_(0),
              shared_space_size_at_last_gc_(0) {
  thread_list_ = new VMThreadList(this);
  interpreter_ = new interpreter::Interpreter(main_isolate);

#ifdef USE_SIMULATOR
  simulator_redirection_ = nullptr;
#endif
#define AllocLock(id, name, msg) \
    locks_[k##id] = new sync::Mutex(msg);
    GLOBAL_NORMAL_LOCK_LIST(AllocLock);
#undef AllocLock
#define AllocLock(id, name, msg) \
    locks_[k##id] = new sync::Mutex(msg, sync::kDefaultMutexLevel, true);
    GLOBAL_RECLUSIVE_LOCK_LIST(AllocLock);
#undef AllocLock
    set_encountered_shared_weak_cells(Smi::kZero);
    set_empty_shared_function_info(Smi::kZero);
}


WorkerRuntime::~WorkerRuntime() {
  delete thread_list_;
  thread_list_ = NULL;

  delete interpreter_;
  interpreter_ = NULL;

#ifdef USE_SIMULATOR
  Simulator::TearDown(nullptr, simulator_redirection_);
  simulator_redirection_ = nullptr;
#endif

  for (int i = 0; i < kGlobalLocksCount; i++) {
    delete(locks_[i]);
    locks_[i] = nullptr;
  }
}

void WorkerRuntime::TearDown() {
  builtins_.TearDown();
  thread_list_->StopAllOtherThreads();
  set_runtime_state(WorkerRuntimeState::kShuttingDown);
}

void WorkerRuntime::IterateStringTableRoots(RootVisitor* v) {
  v->VisitRootPointer(Root::kStringTable,
      reinterpret_cast<Object**>(&string_table_));
}

size_t WorkerRuntime::SharedSinceLastGc() {
  return main_isolate_->heap()->SharedSpaceSizeOfObjects() -
    shared_space_size_at_last_gc_;
}

void WorkerRuntime::IterateRuntimeRoots(RootVisitor* v) {
  v->VisitRootPointer(Root::kStrongRoots, &empty_shared_function_info_);
}

void SharedExternalStringTable::AddString(String* string) {
  DCHECK(string->IsExternalString());
  DCHECK(string->is_shared_object());
  base::LockGuard<base::Mutex> lock_guard(&mutex_);
  shared_old_space_strings_.Add(string);
}

void SharedExternalStringTable::IterateAll(RootVisitor* v) {
  if (!shared_old_space_strings_.is_empty()) {
    Object** start = &shared_old_space_strings_[0];
    v->VisitRootPointers(Root::kExternalStringsTable, start,
                         start + shared_old_space_strings_.length());
  }
}

void SharedExternalStringTable::Verify() {
#ifdef DEBUG
  for (int i = 0; i < shared_old_space_strings_.length(); ++i) {
    Object* obj = Object::cast(shared_old_space_strings_[i]);
    DCHECK(!obj->IsTheHole(worker_runtime_->main_isolate()));
  }
#endif
}

void SharedExternalStringTable::CleanUpAll() {
  int last = 0;
  Isolate* isolate = worker_runtime_->main_isolate();
  for (int i = 0; i < shared_old_space_strings_.length(); ++i) {
    Object* o = shared_old_space_strings_[i];
    if (o->IsTheHole(isolate)) {
      continue;
    }
    if (o->IsThinString()) {
      o = ThinString::cast(o)->actual();
      if (!o->IsExternalString()) continue;
    }
    DCHECK(o->IsExternalString());
    shared_old_space_strings_[last++] = o;
  }
  shared_old_space_strings_.Rewind(last);
  shared_old_space_strings_.Trim();
#ifdef VERIFY_HEAP
  if (FLAG_verify_heap) {
    Verify();
  }
#endif
}

void SharedExternalStringTable::TearDown() {
  Isolate* isolate = worker_runtime_->main_isolate();
  Heap* heap = isolate->heap();
  for (int i = 0; i < shared_old_space_strings_.length(); ++i) {
    Object* o = shared_old_space_strings_[i];
    if (o->IsThinString()) {
      o = ThinString::cast(o)->actual();
      if (!o->IsExternalString()) continue;
    }
    heap->FinalizeExternalString(ExternalString::cast(o));
  }
  shared_old_space_strings_.Free();
}

}  // namespace internal
}  // namespace v8
