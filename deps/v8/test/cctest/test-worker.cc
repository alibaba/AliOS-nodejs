/**
 * {{{ Copyright (C) 2017 The YunOS Project. All rights reserved. }}}
 */
#include <stdlib.h>

#include "src/v8.h"

#include "src/api.h"
#include "src/frames-inl.h"
#include "src/string-stream.h"
#include "test/cctest/cctest.h"
#include "src/thread/worker-runtime.h"
#include "src/objects-inl.h"
#include "src/factory.h"
#include "src/global-handles.h"

using namespace v8;

TEST(CreateWorker) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();

  Isolate::CreateParams params;
  isolate->CreateNewThreadWorker(params, WorkerStartCallback());
}

void worker_callback(Isolate* isolate, void* data) {
  USE(data);
  LocalContext env(isolate);
  HandleScope scope(isolate);
  const char* source = "(function() {return 1;})()";
  Local<Script> script = v8_compile(source);
  Local<Value> result = script->Run(env.local()).ToLocalChecked();
  CHECK(result->IsInt32());
  CHECK_EQ(1, result->Int32Value(env.local()).FromJust());
}

TEST(RunWorker) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();

  Isolate::CreateParams params;
  isolate->CreateNewThreadWorker(params, WorkerStartCallback(
            worker_callback, nullptr));
}


static i::Handle<i::Object> string_array[2];

void worker_callback1(Isolate* isolate, void* data) {
  USE(data);
  i::Isolate* i_isolate = reinterpret_cast<i::Isolate*>(isolate);
  LocalContext env(isolate);
  HandleScope scope(isolate);
  v8::ApiObjectAccessScope aoa(isolate);
  i::Factory* factory = i_isolate->factory();

  i::WorkerRuntime* worker_runtime = i_isolate->worker_runtime();
  i::Isolate* main_isolate = worker_runtime->main_isolate();
  CHECK_EQ(i_isolate->heap()->undefined_value(), main_isolate->heap()->undefined_value());

  i::Handle<i::Object> test_string = factory->InternalizeUtf8String("test");
  CHECK(*test_string == *string_array[0]);
  i::Handle<i::Object> a_string = factory->InternalizeUtf8String("a");
  CHECK(*a_string == *string_array[1]);
}

TEST(CheckSharedObject) {
  LocalContext env;
  v8::Isolate* isolate = env->GetIsolate();
  HandleScope scope(isolate);
  i::Isolate* i_isolate = reinterpret_cast<i::Isolate*>(isolate);
  i::Factory* factory = i_isolate->factory();
  i::GlobalHandles* global_handles = i_isolate->global_handles();

  i::Handle<i::Object> test_string = factory->InternalizeUtf8String("test");
  i::Handle<i::Object> a_string = factory->InternalizeUtf8String("a");
  string_array[0] = global_handles->Create(*test_string);
  string_array[1] = global_handles->Create(*a_string);

  Isolate::CreateParams params;
  isolate->CreateNewThreadWorker(params, WorkerStartCallback(
            worker_callback1, nullptr));
}
