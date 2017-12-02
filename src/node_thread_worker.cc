/**
 * Copyright (C) 2017 Alibaba Group Holding Limited. All Rights Reserved.
 */
#include <list>
#include <algorithm>

#include "node.h"
#include "env-inl.h"
#include "node_thread_worker.h"
#include "unistd.h"

namespace node {
namespace worker {

using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::Value;
using v8::Context;
using v8::Message;
using v8::Function;
using v8::WorkerSerialization;
using v8::HandleScope;
using v8::External;
using v8::FunctionTemplate;
using v8::String;
using v8::Array;
using v8::TryCatch;

const static int kMaxWorkers = 50;
static Mutex workers_mutex_;
static bool allow_new_workers_ = true;
static std::list<Worker*> workers_;

v8::WorkerSerialization* Worker::worker_serialization_(nullptr);

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

static void Throw(Isolate* isolate, const char* message) {
  Environment* env = Environment::GetCurrent(isolate);
  env->ThrowError(message);
}

Worker* GetWorkerFromInternalField(Isolate* isolate, Local<Object> object) {
  if (object->InternalFieldCount() != 1) {
    Throw(isolate, "this is not a Worker");
    return NULL;
  }

  Worker* worker =
      static_cast<Worker*>(object->GetAlignedPointerFromInternalField(0));
  if (worker == NULL) {
    Throw(isolate, "Worker is defunct because it is terminated.");
    return NULL;
  }

  return worker;
}


void SendDataQueue::Enqueue(SendDataType data) {
  Mutex::ScopedLock scoped_lock(mutex_);
  queue_.push(data);
}


bool SendDataQueue::Dequeue(SendDataType* data) {
  Mutex::ScopedLock scoped_lock(mutex_);
  *data = NULL;
  if (queue_.empty()) {
    return false;
  }
  *data = queue_.front();
  queue_.pop();
  return true;
}


bool SendDataQueue::IsEmpty() {
  Mutex::ScopedLock scoped_lock(mutex_);
  bool is_empty = queue_.empty();
  return is_empty;
}

void SendDataQueue::Clear(WorkerSerialization* worker_serialization) {
  Mutex::ScopedLock scoped_lock(mutex_);
  size_t size = queue_.size();
  for (size_t i = 0; i < size; ++i) {
    SendDataType data = queue_.front();
    if (data != nullptr) {
      worker_serialization->CleanupSerializationData(data);
    }
    queue_.pop();
  }
}


static void WorkerNew(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  if (args.Length() < 1 || !args[0]->IsString()) {
    Throw(args.GetIsolate(), "1st argument must be string");
    return;
  }

  if (!args.IsConstructCall()) {
    Throw(args.GetIsolate(), "Worker must be constructed with new");
    return;
  }

  Mutex::ScopedLock scoped_lock(workers_mutex_);
  if (workers_.size() >= kMaxWorkers) {
    Throw(args.GetIsolate(), "Too many workers, I won't let you create more");
    return;
  }

  if (!allow_new_workers_) {
    return;
  }
  // Initialize the internal field to NULL; if we return early without
  // creating a new Worker (because the main thread is terminating) we can
  // early-out from the instance calls.
  args.Holder()->SetAlignedPointerInInternalField(0, NULL);


  String::Utf8Value script(args[0]);
  if (!*script) {
    Throw(args.GetIsolate(), "Can't get worker script");
    return;
  }

  Worker* worker = new Worker(isolate, args.Holder());
  args.Holder()->SetAlignedPointerInInternalField(0, worker);
  workers_.push_back(worker);

  worker->StartExecuteInThread(isolate, *script);
}


static void WorkerPostMessage(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);

  if (args.Length() < 1) {
    Throw(isolate, "Invalid argument");
    return;
  }

  Worker* worker = GetWorkerFromInternalField(isolate, args.Holder());
  if (!worker) {
    return;
  }
  if (worker->IsTerminated()) {
    return;
  }

  SendDataType data = nullptr;
  Local<Value> message = args[0];
  if (args.Length() >= 2) {
    if (!args[1]->IsArray()) {
      Throw(isolate, "Transfer list must be an Array");
      return;
    }
    Local<Array> transfer = Local<Array>::Cast(args[1]);
    data = Worker::GetWorkerSerialization()->SerializeValue(isolate, message, transfer);
  } else {
    data = Worker::GetWorkerSerialization()->SerializeValue(isolate, message);
  }
  if (data != nullptr) {
    worker->PostMessage(data);
  }
}


static void WorkerTerminate(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  Worker* worker = GetWorkerFromInternalField(isolate, args.Holder());
  if (!worker) {
    return;
  }
  worker->Terminate();
}


Worker::Worker(Isolate* isolate, v8::Local<v8::Object> object)
    : script_(nullptr),
      state_(Starting),
      worker_uv_loop_(nullptr),
      master_isolate_(isolate),
      worker_isolate_(nullptr),
      worker_wrapper_(isolate, object) {
}


Worker::~Worker() {
  delete[] script_;
  script_ = nullptr;
  in_queue_.Clear(worker_serialization_);
  out_queue_.Clear(worker_serialization_);
  error_queue_.Clear(worker_serialization_);

  {
    // Clear JS worker object's internal field, avoiding to access illegal native Worker
    // object after the native Worker is released.
    HandleScope handle_scope(master_isolate_);
    v8::Local<v8::Object> wrap_obj = PersistentToLocal(master_isolate_, worker_wrapper_);
    wrap_obj->SetAlignedPointerInInternalField(0, NULL);
    worker_wrapper_.Reset();
  }

  master_isolate_ = nullptr;
  worker_isolate_ = nullptr;

  delete worker_uv_loop_;
  worker_uv_loop_ = nullptr;
}


void Worker::MasterOnMessageCallback(uv_async_t* async) {
  Worker* worker = ContainerOf(&Worker::master_message_async_, async);
  worker->MasterOnMessage(worker->master_isolate_);
}

void Worker::WorkerOnMessageCallback(uv_async_t* async) {
  Worker* worker = ContainerOf(&Worker::worker_message_async_, async);
  worker->OnMessage(worker->worker_isolate_);
}

void Worker::MasterOnErrorCallback(uv_async_t* async) {
  Worker* worker = ContainerOf(&Worker::master_error_async_, async);
  worker->MasterOnError(worker->master_isolate_);
  worker->Terminate();
}

void Worker::UnregisterWorkerCallback(uv_handle_t* handle) {
  Worker* worker = ContainerOf(&Worker::master_message_async_,
      reinterpret_cast<uv_async_t*>(handle));
  Mutex::ScopedLock scoped_lock(workers_mutex_);
  std::list<Worker*>::iterator it = std::find(workers_.begin(), workers_.end(), worker);
  if (it != workers_.end()) {
    workers_.erase(it);
  }
  delete worker;
}

void Worker::MasterOnMessage(Isolate* isolate) {
  HandleScope handle_scope(isolate);

  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> wrap_obj = PersistentToLocal(isolate, worker_wrapper_);
  Local<Value> callback = wrap_obj->Get(env->onmessage_string());
  bool is_function = callback->IsFunction();

  SendDataType data = NULL;
  while (out_queue_.Dequeue(&data)) {
    if (data == NULL) {
      // worker tell us its receive terminated.
      uv_unref((uv_handle_t*)&master_message_async_);
      uv_close((uv_handle_t*)(&master_message_async_), Worker::UnregisterWorkerCallback);

      uv_unref((uv_handle_t*)&master_error_async_);
      uv_close((uv_handle_t*)(&master_error_async_), NULL);

      return;
    }
    Local<Function> onmessage_fun = Local<Function>::Cast(callback);
    if (!is_function) return;
    Local<Value> value;
    if (GetWorkerSerialization()->DeserializeValue(isolate, data).ToLocal(&value)) {
      Local<Object> event = Object::New(isolate);
      event->Set(env->data_string(), value);
      Local<v8::Value> argv[] = {event};
      (void)onmessage_fun->Call(context, wrap_obj, 1, argv);
    }
  }
}

void Worker::MasterOnError(Isolate* isolate) {
  HandleScope handle_scope(isolate);

  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> wrap_obj = PersistentToLocal(isolate, worker_wrapper_);
  Local<Value> callback = wrap_obj->Get(env->onerror_string());
  bool is_function = callback->IsFunction();

  SendDataType data = NULL;
  while (error_queue_.Dequeue(&data)) {
    Local<Function> onerror_fun = Local<Function>::Cast(callback);
    if (!is_function) return;
    Local<Value> error_event;
    if (GetWorkerSerialization()->DeserializeValue(isolate, data).ToLocal(&error_event)) {
      Local<Value> argv[] = {error_event};
      (void)onerror_fun->Call(context, wrap_obj, 1, argv);
    }
  }
}

void Worker::HandleException(Isolate* isolate, TryCatch* try_catch) {
  HandleScope handle_scope(isolate);

  v8::String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = ToCString(exception);
  Local<Message> message = try_catch->Message();

  Local<Object> error_event = Object::New(isolate);

  v8::Local<v8::Context> context(isolate->GetCurrentContext());
  // Print (filename):(line number): (message).
  if (!message.IsEmpty()) {
    // v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
    // const char* filename_string = ToCString(filename);
    error_event->Set(
        String::NewFromUtf8(isolate, "filename", v8::NewStringType::kNormal)
            .ToLocalChecked(), message->GetScriptOrigin().ResourceName());

    int linenum = message->GetLineNumber(context).FromJust();

    // Print line of source code.
    v8::String::Utf8Value sourceline(
        message->GetSourceLine(context).ToLocalChecked());
    error_event->Set(
        String::NewFromUtf8(isolate, "lineno", v8::NewStringType::kNormal)
            .ToLocalChecked(), v8::Number::New(isolate, linenum));

  }

  error_event->Set(
      String::NewFromUtf8(isolate, "message", v8::NewStringType::kNormal)
          .ToLocalChecked(),
      String::NewFromUtf8(isolate, exception_string, v8::NewStringType::kNormal)
          .ToLocalChecked());

  SendDataType data = GetWorkerSerialization()->SerializeValue(isolate, error_event);
  if (data != NULL) {
    error_queue_.Enqueue(data);
    uv_async_send(&(master_error_async_));
  }

  v8::Local<v8::Value> stack_trace_string;
  if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
      stack_trace_string->IsString() &&
      v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
    v8::String::Utf8Value stack_trace(stack_trace_string);
    const char* stack_trace_string = ToCString(stack_trace);
    fprintf(stderr, "%s", stack_trace_string);
  }
}

void Worker::OnMessage(Isolate* isolate) {
  if (GetState() >= Terminated) {
    return;
  }
  HandleScope scope(isolate);

  Environment* env = Environment::GetCurrent(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> global = context->Global();
  Local<Value> callback = global->Get(env->onmessage_string());
  bool is_function = callback->IsFunction();

  SendDataType data = NULL;
  while (in_queue_.Dequeue(&data)) {
    if (data == NULL) {
      CloseCommon();
      NodeInstanceBaseLooperQuit();
      return;
    }
    if (!is_function) return;
    Local<Function> onmessage_fun = Local<Function>::Cast(callback);
    Local<Value> value;
    if (GetWorkerSerialization()->DeserializeValue(isolate, data).ToLocal(&value)) {
      Local<Object> event = Object::New(isolate);
      event->Set(env->data_string(), value);
      Local<Value> argv[] = {event};

      TryCatch try_catch(isolate);
      v8::MaybeLocal<Value> maybe_result = onmessage_fun->Call(context, global, 1, argv);
      Local<Value> result;
      if (!maybe_result.ToLocal(&result)) {
        CHECK(try_catch.HasCaught());
        HandleException(isolate, &try_catch);
      }
    }
  }
}


void Worker::WorkerLoadedCallback(Environment* env, void* data) {
  Worker* worker = reinterpret_cast<Worker*>(data);

  Isolate* isolate = env->isolate();
  HandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> global = context->Global();
  Local<Value> this_value = External::New(isolate, worker);
  Local<FunctionTemplate> postmessage_fun_template =
    FunctionTemplate::New(isolate, Worker::PostMessageOut, this_value);

  Local<Function> postmessage_fun;
  if (postmessage_fun_template->GetFunction(context)
      .ToLocal(&postmessage_fun)) {
    Local<String> name = FIXED_ONE_BYTE_STRING(isolate, "postMessage");
    global->Set(context, name, postmessage_fun).FromJust();
  }


  Local<FunctionTemplate> close_fun_template =
    FunctionTemplate::New(isolate, Worker::CloseInWorker, this_value);

  Local<Function> close_fun;
  if (close_fun_template->GetFunction(context)
      .ToLocal(&close_fun)) {
    Local<String> name = FIXED_ONE_BYTE_STRING(isolate, "close");
    global->Set(context, name, close_fun).FromJust();
  }
  env->process_object()->ForceSet(OneByteString(isolate, "_isWorker"),
      v8::True(isolate), v8::ReadOnly);
}


void Worker::ExecuteInThread(Isolate* isolate, void* data) {
  Worker* worker = reinterpret_cast<Worker*>(data);
  worker->worker_uv_loop_ = uv_loop_new();
  uv_async_init(worker->worker_uv_loop_, &worker->worker_message_async_,
                reinterpret_cast<uv_async_cb>(Worker::WorkerOnMessageCallback));

  if (worker->UpdateState(Running)) {
    // worker is ready, send in_queue data if it has.
    uv_async_send(&worker->worker_message_async_);
    StartWorkerInstance(isolate, worker);
  } else {
    worker->CloseCommon();
  }
  // Post NULL to tell master we are not running.
  worker->out_queue_.Enqueue(NULL);
  uv_async_send(&worker->master_message_async_);
}


void Worker::StartExecuteInThread(Isolate* isolate, const char* script) {
  Environment* env = Environment::GetCurrent(isolate);
  uv_async_init(env->event_loop(), &master_message_async_,
                reinterpret_cast<uv_async_cb>(MasterOnMessageCallback));

  uv_async_init(env->event_loop(), &master_error_async_,
                reinterpret_cast<uv_async_cb>(MasterOnErrorCallback));

  script_ = strdup(script);
  Isolate::CreateParams params;
  worker_isolate_ = isolate->CreateNewThreadWorker(params, v8::WorkerStartCallback(
    ExecuteInThread, reinterpret_cast<void*>(this)));
}


void Worker::PostMessage(SendDataType data) {
  in_queue_.Enqueue(data);
  WorkerState state = GetState();
  if (Running <= state && state <= Terminating) {
    Mutex::ScopedLock scoped_lock(worker_async_mutex_);
    if (uv_is_closing((uv_handle_t*)(&worker_message_async_)) == 0) {
      uv_async_send(&worker_message_async_);
    } else {
      fprintf(stderr, "%s", "worker has been terminated.");
    }
  }
}

// Worker's state must be updated to a higher state.
// It would fail to update to a state that is non-higher than the current state.
// If failed, the state of worker may have been updated by another thread
// to a higher state than the state being update to.
bool Worker::UpdateState(WorkerState state) {
  bool done = false;
  do {
    WorkerState old_state = state_.load(std::memory_order_acquire);
    if (old_state < state) {
      done = state_.compare_exchange_strong(old_state, state);
    } else {
      return false;
    }
  } while (!done);
  return true;
}

void Worker::Terminate() {
  // Post NULL to tell worker we are terminated.
  PostMessage(NULL);
  UpdateState(Terminating);
}

void Worker::PostMessageOut(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);

  if (args.Length() < 1) {
    Throw(isolate, "Invalid argument");
    return;
  }

  CHECK(args.Data()->IsExternal());
  Local<External> this_value = Local<External>::Cast(args.Data());
  Worker* worker = static_cast<Worker*>(this_value->Value());
  SendDataType data = nullptr;
  Local<Value> message = args[0];
  if (args.Length() >= 2) {
    if (!args[1]->IsArray()) {
      Throw(isolate, "Transfer list must be an Array");
      return;
    }
    Local<Array> transfer = Local<Array>::Cast(args[1]);
    data = GetWorkerSerialization()->SerializeValue(isolate, message, transfer);
  } else {
    data = GetWorkerSerialization()->SerializeValue(isolate, message);
  }
  if (data != nullptr) {
    worker->out_queue_.Enqueue(data);
    uv_async_send(&worker->master_message_async_);
  }
}

void Worker::CloseCommon() {
  // work terminated
  {
    Mutex::ScopedLock scoped_lock(worker_async_mutex_);
    uv_close((uv_handle_t*)(&worker_message_async_), NULL);
  }
  UpdateState(Terminated);
}

void Worker::Close() {
  if (UpdateState(Terminating) == false) {
    return;
  }
  CloseCommon();
  NodeInstanceBaseLooperQuit();
}

void Worker::CloseInWorker(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  CHECK(args.Data()->IsExternal());
  Local<External> this_value = Local<External>::Cast(args.Data());
  Worker* worker = static_cast<Worker*>(this_value->Value());
  worker->Close();
}

void Worker::CleanupWorkers(Isolate* isolate) {
  std::list<Worker*> workers_copy;
  {
    Mutex::ScopedLock scoped_lock(workers_mutex_);
    allow_new_workers_ = false;
    workers_copy.swap(workers_);
  }

  for (auto it = workers_copy.cbegin(); it != workers_copy.cend(); ++it) {
    Worker* worker = *it;
    worker->Terminate();
  }
  isolate->JoinAll();

  for (auto it = workers_copy.cbegin(); it != workers_copy.cend(); ++it) {
    Worker* worker = *it;
    delete worker;
  }

  v8::WorkerSerialization* worker_serialization = GetWorkerSerialization();
  worker_serialization->CleanupExternalizedSharedContents();
  delete worker_serialization;
  SetWorkerSerialization(nullptr);
}


void InitWorker(Local<v8::Object> target, Local<v8::Value> unused,
                Local<v8::Context> context) {
  Environment* env = Environment::GetCurrent(context);

  Local<String> class_name = FIXED_ONE_BYTE_STRING(env->isolate(), "ThreadWorker");
  Local<FunctionTemplate> t = env->NewFunctionTemplate(WorkerNew);
  t->SetClassName(class_name);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  t->ReadOnlyPrototype();

  env->SetProtoMethod(t, "postMessage", WorkerPostMessage);
  env->SetProtoMethod(t, "terminate", WorkerTerminate);

  target->Set(class_name, t->GetFunction());
}
}  // namespace worker
}  // namespace node

NODE_MODULE_CONTEXT_AWARE_BUILTIN(thread_worker, node::worker::InitWorker)

