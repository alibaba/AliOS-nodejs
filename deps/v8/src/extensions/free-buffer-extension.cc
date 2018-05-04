// Copyright 2013 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/extensions/free-buffer-extension.h"

#include "src/api.h"
#include "src/base/platform/platform.h"
#include "src/isolate.h"
#include "src/objects-inl.h"
#include "src/api.h"

namespace v8 {
namespace internal {


v8::Local<v8::FunctionTemplate> FreeBufferExtension::GetNativeFunctionTemplate(
    v8::Isolate* isolate, v8::Local<v8::String> str) {
  return v8::FunctionTemplate::New(isolate, FreeBufferExtension::FreeBuffer);
}


void FreeBufferExtension::FreeBuffer(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Local<v8::ArrayBuffer> arrayBuffer = args[0].As<v8::ArrayBuffer>();
  v8::ArrayBuffer::Contents contents = arrayBuffer->Externalize();
  Isolate* isolate = reinterpret_cast<Isolate*>(args.GetIsolate());
  // <YUNOS> START:
  const int allocator_id = (*Utils::OpenHandle(*arrayBuffer))->allocator_id();
  v8::ArrayBuffer::Allocator* allocator = isolate->array_buffer_allocator(allocator_id);
  i::Handle<i::Object> obj = Utils::OpenHandle(*arrayBuffer);
  if (JSArrayBuffer::cast(*obj)->is_shared_process()) {
    ArrayBuffer::Allocator::FreeShared(contents.Data(), contents.ByteLength());
  } else {
    allocator->Free(contents.Data(), contents.ByteLength());
  }
  // <YUNOS> END
}

}  // namespace internal
}  // namespace v8
