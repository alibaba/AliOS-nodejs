/**
 * Copyright (C) 2017 Alibaba Group Holding Limited. All Rights Reserved.
 */
#ifndef SRC_THREAD_NODE_WORKER_H_
#define SRC_THREAD_NODE_WORKER_H_

#include "node.h"
#include "v8.h"
#include "uv.h"
#include "node_mutex.h"

#include <queue>
#include <atomic>

namespace node {
namespace worker {

typedef v8::WorkerSerializationData* SendDataType;

class SendDataQueue {
 public:
  SendDataQueue() {}
  ~SendDataQueue() {}

  void Enqueue(SendDataType data);
  bool Dequeue(SendDataType* data);
  bool IsEmpty();
  void Clear(v8::WorkerSerialization* worker_serialization);

 private:
  Mutex mutex_;
  std::queue<SendDataType> queue_;
};


class Worker {
 public:
  enum WorkerState {
    // Starting state is from new Worker() in master to
    // uv_async_init(worker_message_async_) in worker thread.
    // In this state, Worker object is created by Master,
    // but worker thread hasn't started or initialized yet.
    // Master can pending message to worker, bu worker won't handle it.
    Starting,
    // Running state is from initialization of worker_message_async_ in worker thread
    // to worker.terminate() in master or close() in worker.
    // Worker is running in this state.
    // worker_message_async_ is active.
    Running,
    // Reserve for testing. In this state, master has disconnected with worker
    // and can't post any message to worker.
    // Worker end up by itself after completing all its tasks.
    RunningFreely,
    // Terminating state is from worker.terminate() in master or close() in worker
    // to uv_close(worker_message_async_).
    // In this state, worker is ordered to terminate by master, or itself.
    // Master can't post any message to worker but the last 'NULL' message.
    // Worker will only handle the last 'NULL' message from master
    // and discard all other messages in its loop.
    Terminating,
    // Terminated state is from  uv_close(worker_message_async_) to the end.
    // In this state, worker goes to end.
    // worker_message_async_ is closed.
    Terminated,
  };

  Worker(v8::Isolate* isolate, v8::Local<v8::Object> object);
  ~Worker();

  void StartExecuteInThread(v8::Isolate* isolate, const char* script);
  void PostMessage(SendDataType data);
  void Terminate();
  void Close();
  inline bool IsTerminated() const {
    return state_.load(std::memory_order_acquire) >= Terminating;
  }
  inline bool IsRunning() {
    return state_.load(std::memory_order_acquire);
  }
  inline WorkerState GetState() const {
    return state_.load(std::memory_order_acquire);
  }
  bool UpdateState(WorkerState state);
  uv_loop_t* event_loop() { return worker_uv_loop_; }
  char* script() { return script_; }

  static void WorkerLoadedCallback(Environment* env, void* data);
  static void MasterOnMessageCallback(uv_async_t* async);
  static void WorkerOnMessageCallback(uv_async_t* async);
  static void MasterOnErrorCallback(uv_async_t* async);
  static void CleanupWorkers(v8::Isolate* isolate);
  static inline void SetWorkerSerialization(v8::WorkerSerialization* worker_serialization) {
    worker_serialization_ = worker_serialization;
  }
  static inline v8::WorkerSerialization* GetWorkerSerialization() {
    return worker_serialization_;
  }

 private:
  void MasterOnMessage(v8::Isolate* isolate);
  void OnMessage(v8::Isolate* isolate);
  void MasterOnError(v8::Isolate* isolate);
  void HandleException(v8::Isolate* isolate, v8::TryCatch* try_catch);
  void CloseCommon();

  static void PostMessageOut(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void CloseInWorker(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ExecuteInThread(v8::Isolate* isolate, void* data);
  static void UnregisterWorkerCallback(uv_handle_t* handle);

  SendDataQueue in_queue_;
  SendDataQueue out_queue_;
  SendDataQueue error_queue_;
  char* script_;
  // Refer to WorkerState enum for more details.
  std::atomic<WorkerState> state_;
  // Guard worker_message_async_ so as
  // not to uv_async_send and uv_close at the time time.
  Mutex worker_async_mutex_;
  uv_async_t worker_message_async_;
  uv_async_t master_message_async_;
  uv_async_t master_error_async_;

  uv_loop_t* worker_uv_loop_;

  v8::Isolate* master_isolate_;
  v8::Isolate* worker_isolate_;
  v8::Persistent<v8::Object> worker_wrapper_;

  // Shared by master and all worker threads.
  static v8::WorkerSerialization* worker_serialization_;
};


}  // namespace worker
}  // namespace node

#endif  // SRC_NODE_THREAD_WORKER_H_
