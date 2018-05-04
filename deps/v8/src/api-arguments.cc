// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/api-arguments.h"

#include "src/debug/debug.h"
#include "src/objects-inl.h"
#include "src/tracing/trace-event.h"
#include "src/vm-state-inl.h"

namespace v8 {
namespace internal {

Handle<Object> FunctionCallbackArguments::Call(FunctionCallback f) {
  Isolate* isolate = this->isolate();
  if (isolate->needs_side_effect_check() &&
      !isolate->debug()->PerformSideEffectCheckForCallback(FUNCTION_ADDR(f))) {
    return Handle<Object>();
  }
  RuntimeCallTimerScope timer(isolate, &RuntimeCallStats::FunctionCallback);
  ExternalCallbackScope call_scope(isolate, FUNCTION_ADDR(f));
  FunctionCallbackInfo<v8::Value> info(begin(), argv_, argc_);
  {
    VMState<EXTERNAL> state(isolate);
    f(info);
  }
  return GetReturnValue<Object>(isolate);
}

Handle<JSObject> PropertyCallbackArguments::Call(
    IndexedPropertyEnumeratorCallback f) {
  Isolate* isolate = this->isolate();
  if (isolate->needs_side_effect_check() &&
      !isolate->debug()->PerformSideEffectCheckForCallback(FUNCTION_ADDR(f))) {
    return Handle<JSObject>();
  }
  RuntimeCallTimerScope timer(isolate, &RuntimeCallStats::PropertyCallback);
  ExternalCallbackScope call_scope(isolate, FUNCTION_ADDR(f));
  PropertyCallbackInfo<v8::Array> info(begin());
  {
    VMState<EXTERNAL> state(isolate);
    f(info);
  }
  return GetReturnValue<JSObject>(isolate);
}

bool PropertyCallbackArguments::PerformSideEffectCheck(Isolate* isolate,
                                                       Address function) {
  return isolate->debug()->PerformSideEffectCheckForCallback(function);
}

}  // namespace internal
}  // namespace v8
