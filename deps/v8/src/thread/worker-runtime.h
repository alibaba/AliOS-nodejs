/**
 * {{{ Copyright (C) 2017 The YunOS Project. All rights reserved. }}}
 */

#ifndef THREAD_WORKER_RUNTIME_H_
#define THREAD_WORKER_RUNTIME_H_

#include "src/base/platform/platform.h"
#include "src/base/logging.h"
#include "src/thread/sync/mutex.h"
#include "src/handles.h"
#include "src/builtins/builtins.h"
#include "src/list.h"
#include "src/base/platform/mutex.h"

namespace v8 {
namespace internal {

class VMThreadList;
class Isolate;
class Object;
class String;
class StringTable;
class RootVisitor;
class Redirection;
class GCTracer;
class SharedFunctionInfo;

namespace interpreter {
  class Interpreter;
}

enum class WorkerRuntimeState {
  kUnInitialize = 0, // the runtime is not initizlied
  kInitialized,  // runtime is ready to run
  kSuspend,      // runtime is be checked out
  kShuttingDown, // runtime has been shut down
};

#define GLOBAL_NORMAL_LOCK_LIST(C) \
  C(SharedHeap, shared_heap, "shared heap") \
  C(WeakCells, weak_cells, "encountered weak cells") \
  C(ScriptList, script_list, "script list") \
  C(NoScriptSharedFunctionInfos, no_script_shared_function_infos, \
    "no script shared function infos")


#define GLOBAL_RECLUSIVE_LOCK_LIST(C) \
  C(StringTable, string_table, "string table lock") \
  C(CodeStubs, code_stubs, "string table lock")


#define GLOBAL_LOCK_LIST(C) \
    GLOBAL_NORMAL_LOCK_LIST(C) \
    GLOBAL_RECLUSIVE_LOCK_LIST(C)

class ThreadIterator {
 public:
  ThreadIterator(VMThreadList* threads, VMThread* self);
  ~ThreadIterator();
  bool Done() { return cur_ == count_; }
  VMThread* GetCurrent() const { return current_; }
  void Advance();
  VMThread* self() {return self_;}

 private:
  int cur_;
  int count_;
  bool lock_;
  VMThread* current_;
  VMThreadList* thread_list_;
  VMThread* self_;
};

class WorkerRuntime;

class SharedExternalStringTable {
 public:
  SharedExternalStringTable(WorkerRuntime* worker_runtime)
      : worker_runtime_(worker_runtime) {}

  void AddString(String* string);
  void IterateAll(RootVisitor* v);
  void CleanUpAll();
  void TearDown();
  void Verify();

 private:
  WorkerRuntime*  worker_runtime_;
  List<Object*> shared_old_space_strings_;
  base::Mutex mutex_;

  DISALLOW_COPY_AND_ASSIGN(SharedExternalStringTable);
  friend class WorkerRuntime;
};

class WorkerRuntime {
 public:
  enum GlobalLockKind {
#define ENUM_LOCKS(id, name, msg) k##id,
    GLOBAL_LOCK_LIST(ENUM_LOCKS)
#undef ENUM_LOCKS
    kGlobalLocksCount,
  };

  // TLS key used to retrieve the WorkerRuntime
  V8_EXPORT_PRIVATE static base::Thread::LocalStorageKey runtime_key_;

  static void InitializeOncePerProcess();
  static bool IsShuttingDown();
  typedef void (*WorkerCallback)(Isolate* isolate);
  Isolate* CreateNewWorker(v8::WorkerStartCallback worker_callback);

  WorkerRuntime(Isolate* main_isolate);
  ~WorkerRuntime();

  static inline WorkerRuntime* Current() {
    void* runtime = base::Thread::GetExistingThreadLocal(
        WorkerRuntime::runtime_key_);
    return reinterpret_cast<WorkerRuntime*>(runtime);
  }

  template <typename Callback>
  void ForAllWorkers(Callback callback) {
    ThreadIterator iterator(this->thread_list(), NULL);
    for (auto thread = iterator.GetCurrent();
          !iterator.Done(); iterator.Advance(), thread = iterator.GetCurrent()) {
      callback(thread);
    }
  }

  template <typename Callback>
  void ForAllWorkersWithOutSelf(Callback callback) {
    ThreadIterator iterator(this->thread_list(), NULL);
    for (auto thread = iterator.GetCurrent();
          !iterator.Done(); iterator.Advance(), thread = iterator.GetCurrent()) {
      if (thread == iterator.self()) continue;
      callback(thread);
    }
  }

  VMThreadList* thread_list() const { return thread_list_; }

  void set_runtime_state(WorkerRuntimeState runtime_state) {
    runtime_state_ = runtime_state;
  }

  WorkerRuntimeState runtime_state() const { return runtime_state_; }
  Isolate* main_isolate() const { return main_isolate_; }

  void TearDown();

  Builtins* builtins() { return &builtins_; }
  interpreter::Interpreter* interpreter() const { return interpreter_; }

  sync::Mutex* get(GlobalLockKind kind) {
      return locks_[kind];
  }

  static sync::Mutex& GetLock(WorkerRuntime* worker_runtime, GlobalLockKind kind) {
    return *(worker_runtime->get(kind));
  }

  void IterateStringTableRoots(RootVisitor* v);

  Handle<StringTable> string_table() {
    return Handle<StringTable>(&string_table_);
  }

  StringTable* StringTableRoot() const {
    return string_table_;
  }

  void SetStringTableRoot(StringTable* value) {
    string_table_ = value;
  }

#ifdef USE_SIMULATOR
  base::Mutex* simulator_redirection_mutex() { return &simulator_redirection_mutex_; }
  Redirection* simulator_redirection() { return simulator_redirection_; }
  void set_simulator_redirection(Redirection* redirection) {
    simulator_redirection_ = redirection;
  }
#endif

  bool IsJSThread() {
    return Current() != nullptr;
  }

  void set_encountered_shared_weak_cells(Object* weak_cell) {
    encountered_shared_weak_cells_ = weak_cell;
  }

  Object* encountered_shared_weak_cells() {
    return encountered_shared_weak_cells_;
  }

  void set_empty_shared_function_info(Object* info) {
    empty_shared_function_info_ = info;
  }

  Handle<Object> EmptySharedFunctionInfo() {
    return Handle<Object>(&empty_shared_function_info_);
  }

  void RegisterExternalString(String* string) {
    shared_external_string_table_.AddString(string);
  }

  void CleanUpSharedExternalStringTable() {
    shared_external_string_table_.CleanUpAll();
  }

  void IterateSharedExternalStringTable(RootVisitor* v) {
    shared_external_string_table_.IterateAll(v);
  }

  void IterateSharedExternalStringTableWithLock(RootVisitor* v) {
    base::LockGuard<base::Mutex> lock_guard(&shared_external_string_table_.mutex_);
    shared_external_string_table_.IterateAll(v);
  }

  void TearDownSharedExternalStringTable() {
    shared_external_string_table_.TearDown();
  }

  GCTracer* tracer() {
    return tracer_;
  }

  void set_tracer(GCTracer* tracer) {
    tracer_ = tracer;
  }

  void UpdateTotalGCTime(double duration) {
    total_shared_gc_time_ms_ += duration;
  }

  double total_shared_gc_time() {
    return total_shared_gc_time_ms_;
  }

  void UpdateSharedSpacesAllocationCounter() {
    shared_space_allocation_counter_at_last_gc_ =
      SharedSpaceAllocationCounter();
  }

  size_t SharedSpaceAllocationCounter() {
    return shared_space_allocation_counter_at_last_gc_ +
            SharedSinceLastGc();
  }

  void UpdateSharedSpaceSizeAtLastGc(size_t value) {
    shared_space_size_at_last_gc_ = value;
  }

  size_t SharedSinceLastGc();
  void IterateRuntimeRoots(RootVisitor* v);

 private:
  Isolate* main_isolate_;
  VMThreadList* thread_list_;
  WorkerRuntimeState runtime_state_;
  Builtins builtins_;
  interpreter::Interpreter* interpreter_;

  StringTable* string_table_;
  SharedExternalStringTable shared_external_string_table_;
#ifdef USE_SIMULATOR
  Redirection* simulator_redirection_;
  base::Mutex simulator_redirection_mutex_;
#endif

  sync::Mutex* locks_[kGlobalLocksCount];

  /*Heap global structs*/
  // List of shared weak cells, normal weak cells
  // are put in heap_->encountered_weak_cells_
  Object* encountered_shared_weak_cells_;
  Object* empty_shared_function_info_;

  //Some informations are blong to all heaps
  //GcTracer for shared heap
  //we put them here
  GCTracer* tracer_;

  // Total time spent in Shared GC.
  double total_shared_gc_time_ms_;

  // This counter is increased before each GC and never reset. To
  // account for the bytes allocated since the last GC, use the
  // SharedSpaceAllocationCounter() function.
  size_t shared_space_allocation_counter_at_last_gc_;

  // The size of objects in old generation after the last MarkCompact GC.
  size_t shared_space_size_at_last_gc_;
};

}  // namespace internal
}  // namespace v8
#endif  // THREAD_WORKER_RUNTIME_H_
