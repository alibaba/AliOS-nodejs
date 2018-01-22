#include "v8.h"

#include "node_snapshot.h"
#include "node_internals.h"
#include "env.h"
#include "async_wrap.h"
#include "tty_wrap.h"
#include "inspector_agent.h"

#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

namespace v8 {
namespace internal {
  extern bool FLAG_expose_gc;
}

template<>
template <class S>
V8_INLINE
typename std::enable_if<std::is_base_of<Value, S>::value, Local<S>>
Local<Data>::As() const {
  return Local<S>::Cast(*(reinterpret_cast<const Local<Value>*>(this)));
}

}  // namespace v8

namespace node {

using v8::Array;
using v8::ArrayBuffer;
using v8::Boolean;
using v8::Context;
using v8::EscapableHandleScope;
using v8::Exception;
using v8::External;
using v8::Float64Array;
using v8::Function;
using v8::FunctionCallback;
using v8::FunctionCallbackInfo;
using v8::HandleScope;
using v8::HeapStatistics;
using v8::Integer;
using v8::Isolate;
using v8::Just;
using v8::Local;
using v8::Locker;
using v8::Maybe;
using v8::MaybeLocal;
using v8::Message;
using v8::Name;
using v8::NamedPropertyHandlerConfiguration;
using v8::Nothing;
using v8::Null;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::Primitive;
using v8::Private;
using v8::Promise;
using v8::PromiseRejectMessage;
using v8::PropertyCallbackInfo;
using v8::ScriptOrigin;
using v8::SealHandleScope;
using v8::SnapshotCreator;
using v8::String;
using v8::TryCatch;
using v8::TypedArray;
using v8::Uint32Array;
using v8::Uint8Array;
using v8::Undefined;
using v8::V8;
using v8::Value;

void ReConstructTTYWrap(Environment* env,
                        v8::Local<v8::Object> object,
                        int fd,
                        bool readable);

namespace cares_wrap {

void ReConstructChannelWrap(Environment* env, Local<Object> object);

}

#define TEMPLATE_LIST(V)                                                      \
  NODE_BUILTIN_MODULES(V)                                                     \
  V(node)                                                                     \
  V(handlewrap)                                                               \
  V(streambase)                                                               \
  V(statwatcher)

#define V(modname) size_t _templates_##modname(const FunctionCallback**);
  TEMPLATE_LIST(V)
#undef V

#define HTTP2_STATE_FIELDS(V)                                                 \
  V(v8::Uint8Array, root_buffer)                                              \
  V(v8::Float64Array, session_state_buffer)                                   \
  V(v8::Float64Array, stream_state_buffer)                                    \
  V(v8::Float64Array, stream_stats_buffer)                                    \
  V(v8::Float64Array, session_stats_buffer)                                   \
  V(v8::Uint32Array, padding_buffer)                                          \
  V(v8::Uint32Array, options_buffer)                                          \
  V(v8::Uint32Array, settings_buffer)

namespace {

enum ContextIndepentSnapshotIndex : size_t {
  kAsExternal,
#define V(PropertyName, StringValue)  k##PropertyName,
  PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(V)
  PER_ISOLATE_STRING_PROPERTIES(V)
#undef V
  kProviderStringStart,
  kHttp2StaticStrStart = kProviderStringStart + AsyncWrap::PROVIDERS_LENGTH
};

enum ContextDepentSnapshotIndex : size_t {
#define V(PropertyName, TypeName)   k##PropertyName,
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_CONTEXT(V)
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_CONSOLE(V)
#undef V
  kTypedArrayStart,
  kHttp2StateStart = kTypedArrayStart + 6
};

intptr_t* SetupExternalReferences(intptr_t env_addr) {
  // reserve for Environment* and null sentinal
  size_t total_size = 2;

#define V(modname)                                                            \
  const FunctionCallback* modname##_templates;                                \
  size_t modname##_size = _templates_##modname(&modname##_templates);         \
  total_size += modname##_size;
  TEMPLATE_LIST(V)
#undef V

  intptr_t* references = new intptr_t[total_size];
  CHECK_EQ(sizeof(intptr_t), sizeof(const FunctionCallback));

  size_t offset = 0;
  references[offset++] = env_addr;

#define V(modname)                                                            \
  if (modname##_size != 0) {                                                  \
    memcpy(static_cast<void*>(references + offset),                           \
           reinterpret_cast<const void*>(modname##_templates),                \
           modname##_size * sizeof(const FunctionCallback));                  \
    offset += modname##_size;                                                 \
  }
  TEMPLATE_LIST(V)
#undef V

  references[total_size - 1] = 0;

  return references;
}

#undef TEMPLATE_LIST


class NodePersistentHandleVisitor : public v8::PersistentHandleVisitor {
  public:
    NodePersistentHandleVisitor(Environment* env) :
      env_(env) {}

    virtual void VisitPersistentHandle(v8::Persistent<Value>* value,
        uint16_t class_id) override {
      Isolate* isolate = env_->isolate();
      uint16_t new_class_id = class_id - NODE_ASYNC_ID_OFFSET;

      if (new_class_id == AsyncWrap::PROVIDER_DNSCHANNEL) {
        Local<Object> holder = PersistentToLocal(isolate, *value)->ToObject(isolate);
        CHECK(env_->dnschannel().IsEmpty());
        env_->set_dnschannel(holder);
      } else if (new_class_id == AsyncWrap::PROVIDER_TTYWRAP) {
        Local<Object> holder = PersistentToLocal(isolate, *value)->ToObject(isolate);
        void* ptr = holder->GetAlignedPointerFromInternalField(0);
        TTYWrap* wrap = reinterpret_cast<TTYWrap*>(ptr);
        int fd = wrap->fd();
        if (fd == 1) {
          CHECK(env_->ttywrap_stdout().IsEmpty());
          env_->set_ttywrap_stdout(holder);
        } else if (fd == 2) {
          CHECK(env_->ttywrap_stderr().IsEmpty());
          env_->set_ttywrap_stderr(holder);
        } else {
          UNREACHABLE();
        }
      } else if (new_class_id == AsyncWrap::PROVIDER_SIGNALWRAP) {
        Local<Object> holder = PersistentToLocal(isolate, *value)->ToObject(isolate);
        CHECK(env_->signalwrap().IsEmpty());
        env_->set_signalwrap(holder);
      } else {
        printf("%s: %u\n", __FUNCTION__, class_id);
      }
    }

  private:
    Environment* env_;
};

void SetupGlobalConsole(Environment* env, std::vector<AsyncWrap*>* known_ptrs) {
  NodePersistentHandleVisitor visitor(env);
  env->isolate()->VisitHandlesWithClassIds(&visitor);

  Local<Object> holder;
  AsyncWrap* wrap;

#define V(PropertyName, TypeName)                                             \
  holder = env->PropertyName();                                               \
  wrap = reinterpret_cast<AsyncWrap*>(holder                                  \
      ->GetAlignedPointerFromInternalField(0));                               \
  CHECK_NE(wrap, nullptr);                                                    \
  known_ptrs->push_back(wrap);                                                \
  holder->SetAlignedPointerInInternalField(0, nullptr);
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_CONSOLE(V)
#undef V
}

void RestoreGlobalConsole(Environment* env, std::vector<AsyncWrap*>* known_ptrs) {
  Local<Object> holder;
  AsyncWrap* wrap;
  auto iterator = known_ptrs->begin();

#define V(PropertyName, TypeName)                                             \
  holder = env->PropertyName();                                               \
  wrap = *iterator;                                                           \
  CHECK_NE(wrap, nullptr);                                                    \
  holder->SetAlignedPointerInInternalField(0, wrap);                          \
  ++iterator;
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_CONSOLE(V)
#undef V

  CHECK_EQ(iterator, known_ptrs->end());
}

size_t AddContextIndepentObjectsToSnapshot(SnapshotCreator* creator,
                                           Environment* env) {
  Isolate* isolate = creator->GetIsolate();
  size_t num_objs = ContextIndepentSnapshotIndex::kAsExternal;

  CHECK_EQ(num_objs++, creator->AddData(env->as_external()));

#define VP(PropertyName, StringValue) V(v8::Private, PropertyName)
#define VS(PropertyName, StringValue) V(v8::String, PropertyName)
#define V(TypeName, PropertyName)                                             \
  CHECK_EQ(num_objs++, creator->AddData(env->PropertyName()));
  PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(VP)
  PER_ISOLATE_STRING_PROPERTIES(VS)
#undef V
#undef VS
#undef VP

  CHECK_EQ(num_objs, ContextIndepentSnapshotIndex::kProviderStringStart);
  for (size_t i = 0; i < AsyncWrap::PROVIDERS_LENGTH; ++i) {
    CHECK_EQ(num_objs++,
             creator->AddData(env->async_hooks()->provider_string(i)));
  }

  // http2_static_strs
  CHECK_EQ(num_objs, ContextIndepentSnapshotIndex::kHttp2StaticStrStart);
  auto& str_map = env->isolate_data()->http2_static_strs;
  for (auto iter = str_map.begin(); iter != str_map.end(); ++iter) {
    CHECK_EQ(num_objs++, creator->AddData(iter->second.Get(isolate)));
  }

  return num_objs;
}

size_t AddContextDepentObjectsToSnapshot(SnapshotCreator* creator,
                                         Environment* env) {
  Isolate* isolate = creator->GetIsolate();
  Local<Context> context = env->context();
  Local<Primitive> undefined = Undefined(isolate);
  size_t num_objs = 0;

#define V(PropertyName, TypeName)                                             \
  {                                                                           \
    size_t idx;                                                               \
    if (env->PropertyName().IsEmpty()) {                                      \
      idx = creator->AddData(context, undefined);                             \
    } else {                                                                  \
      idx = creator->AddData(context, env->PropertyName());                   \
    }                                                                         \
    CHECK_EQ(num_objs, idx);                                                  \
    ++num_objs;                                                               \
  }
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_CONTEXT(V)
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_CONSOLE(V)
#undef V

  CHECK_EQ(num_objs, ContextDepentSnapshotIndex::kTypedArrayStart);
  // async_hooks, non-empty
  creator->AddData(context, env->async_hooks()->async_ids_stack().GetJSArray());
  creator->AddData(context, env->async_hooks()->fields().GetJSArray());
  creator->AddData(context, env->async_hooks()->async_id_fields().GetJSArray());

  // immediate_info, non-empty
  creator->AddData(context, env->immediate_info()->fields().GetJSArray());

  // tick_info, non-empty
  creator->AddData(context, env->tick_info()->fields().GetJSArray());

  // should_abort_on_uncaught_toggle, non-empty
  creator->AddData(context, env->should_abort_on_uncaught_toggle()
      .GetJSArray());

  CHECK_EQ(num_objs + 6, ContextDepentSnapshotIndex::kHttp2StateStart);

  http2::http2_state* state = env->http2_state();
  // fields of http2::http2_state are non-empty
#define ADD_HTTP2_STATE_FIELD(type, field)                                    \
  creator->AddData(context, state->field.GetJSArray());
  HTTP2_STATE_FIELDS(ADD_HTTP2_STATE_FIELD)
#undef ADD_HTTP2_STATE_FIELD

  return num_objs + 6 + 8;
}

void AddObjectsToSnapshot(SnapshotCreator* creator, Environment* env) {
  AddContextIndepentObjectsToSnapshot(creator, env);
  AddContextDepentObjectsToSnapshot(creator, env);
}

v8::StartupData SerializeInternalFields(Local<Object> holder, int index, void* data) {
  void* ptr = holder->GetAlignedPointerFromInternalField(index);
  if (ptr != nullptr) {
    printf("%s: %p\n", __FUNCTION__, ptr);
  }

  return {nullptr, 0};
}

class SnapshotWriter {
 public:
  SnapshotWriter()
      : snapshot_cpp_path_(nullptr), snapshot_blob_path_(nullptr) {}

  void SetSnapshotFile(const char* snapshot_cpp_file) {
    snapshot_cpp_path_ = snapshot_cpp_file;
  }

  void SetStartupBlobFile(const char* snapshot_blob_file) {
    snapshot_blob_path_ = snapshot_blob_file;
  }

  void WriteSnapshot(v8::StartupData blob) const {
    // TODO(crbug/633159): if we crash before the files have been fully created,
    // we end up with a corrupted snapshot file. The build step would succeed,
    // but the build target is unusable. Ideally we would write out temporary
    // files and only move them to the final destination as last step.
    MaybeWriteSnapshotFile(blob);
    MaybeWriteStartupBlob(blob);
  }

 private:
  void MaybeWriteStartupBlob(v8::StartupData blob) const {
    if (!snapshot_blob_path_) return;

    FILE* fp = GetFileDescriptorOrDie(snapshot_blob_path_);
    size_t written = fwrite(blob.data, 1, blob.raw_size, fp);
    fclose(fp);
    if (written != static_cast<size_t>(blob.raw_size)) {
      printf("Writing snapshot file failed.. Aborting.\n");
      remove(snapshot_blob_path_);
      exit(1);
    }
  }

  void MaybeWriteSnapshotFile(v8::StartupData blob) const {
    if (!snapshot_cpp_path_) return;

    FILE* fp = GetFileDescriptorOrDie(snapshot_cpp_path_);

    WriteFilePrefix(fp);
    WriteData(fp, blob);
    WriteFileSuffix(fp);

    fclose(fp);
  }

  static void WriteFilePrefix(FILE* fp) {
    fprintf(fp, "// Autogenerated snapshot file. Do not edit.\n\n");
    fprintf(fp, "#include \"src/v8.h\"\n");
    fprintf(fp, "#include \"src/base/platform/platform.h\"\n\n");
    fprintf(fp, "#include \"src/snapshot/snapshot.h\"\n\n");
    fprintf(fp, "namespace v8 {\n");
    fprintf(fp, "namespace internal {\n\n");
  }

  static void WriteFileSuffix(FILE* fp) {
    fprintf(fp, "const v8::StartupData* Snapshot::DefaultSnapshotBlob() {\n");
    fprintf(fp, "  return &blob;\n");
    fprintf(fp, "}\n\n");
    fprintf(fp, "}  // namespace internal\n");
    fprintf(fp, "}  // namespace v8\n");
  }

  static void WriteData(FILE* fp, v8::StartupData blob) {
    fprintf(fp, "static const byte blob_data[] = {\n");
    WriteSnapshotData(fp, blob);
    fprintf(fp, "};\n");
    fprintf(fp, "static const int blob_size = %d;\n", blob.raw_size);
    fprintf(fp, "static const v8::StartupData blob =\n");
    fprintf(fp, "{ (const char*) blob_data, blob_size };\n");
  }

  static void WriteSnapshotData(FILE* fp,
                                v8::StartupData blob) {
    for (int i = 0; i < blob.raw_size; i++) {
      if ((i & 0x1f) == 0x1f) fprintf(fp, "\n");
      if (i > 0) fprintf(fp, ",");
      fprintf(fp, "%u", static_cast<unsigned char>(blob.data[i]));
    }
    fprintf(fp, "\n");
  }

  static FILE* GetFileDescriptorOrDie(const char* filename) {
    FILE* fp = fopen(filename, "wb");
    if (fp == nullptr) {
      printf("Unable to open file \"%s\" for writing.\n", filename);
      exit(1);
    }
    return fp;
  }

  const char* snapshot_cpp_path_;
  const char* snapshot_blob_path_;
};

}  // anonymous space

IsolateData::IsolateData(Isolate* isolate,
                         uv_loop_t* event_loop,
                         MultiIsolatePlatform* platform,
                         uint32_t* zero_fill_field,
                         bool from_snapshot) :
#define V(PropertyName, StringValue)                                          \
    PropertyName ## _(                                                        \
        isolate,                                                              \
        isolate->GetDataFromSnapshotOnce<Private>(                            \
          ContextIndepentSnapshotIndex::k##PropertyName).ToLocalChecked()),
    PER_ISOLATE_PRIVATE_SYMBOL_PROPERTIES(V)
#undef V
#define V(PropertyName, StringValue)                                          \
    PropertyName ## _(                                                        \
        isolate,                                                              \
        isolate->GetDataFromSnapshotOnce<String>(                             \
          ContextIndepentSnapshotIndex::k##PropertyName).ToLocalChecked()),
    PER_ISOLATE_STRING_PROPERTIES(V)
#undef V
    isolate_(isolate),
    event_loop_(event_loop),
    zero_fill_field_(zero_fill_field),
    platform_(platform) {
  CHECK(from_snapshot);
  if (platform_ != nullptr)
    platform_->RegisterIsolate(this, event_loop);
}

void ReSetupGlobalConsole(Environment* const env) {
  v8::HandleScope handle_scope(env->isolate());
  Local<Object> empty_object;

  Local<Object> js_stdout = env->ttywrap_stdout();
  ReConstructTTYWrap(env, js_stdout, 1, false);
  env->set_ttywrap_stdout(empty_object);

  Local<Object> js_stderr = env->ttywrap_stderr();
  ReConstructTTYWrap(env, js_stderr, 2, false);
  env->set_ttywrap_stderr(empty_object);

  Local<Object> dnschannel = env->dnschannel();
  cares_wrap::ReConstructChannelWrap(env, dnschannel);
  env->set_dnschannel(empty_object);

  Local<Object> signal = env->signalwrap();
  cares_wrap::ReConstructChannelWrap(env, signal);
  env->set_signalwrap(empty_object);
}

Environment::Environment(IsolateData* isolate_data,
                         Local<Context> context,
                         Local<External> as_external,
                         std::vector<JSTypedArrays>* typed_arrays,
                         std::vector<JSTypedArrays>* http2_state_arrays,
                         std::vector<Local<String>>* async_provider_strings)
    : isolate_(context->GetIsolate()),
      isolate_data_(isolate_data),
      async_hooks_(typed_arrays->at(0).as_float64_array, // async_ids_stack
                   typed_arrays->at(1).as_uint32_array, // fields
                   typed_arrays->at(2).as_float64_array, // async_id_fields
                   async_provider_strings),
      immediate_info_(isolate_, typed_arrays->at(3).as_uint32_array),
      tick_info_(isolate_, typed_arrays->at(4).as_uint8_array),
      timer_base_(uv_now(isolate_data->event_loop())),
      using_domains_(false),
      printed_error_(false),
      trace_sync_io_(false),
      abort_on_uncaught_exception_(false),
      emit_napi_warning_(true),
      makecallback_cntr_(0),
      should_abort_on_uncaught_toggle_(isolate_,
                                       typed_arrays->at(5).as_uint32_array),
#if HAVE_INSPECTOR
      inspector_agent_(new inspector::Agent(this)),
#endif
      handle_cleanup_waiting_(0),
      // FIXME(liqy)
      http_parser_buffer_(nullptr),
      http2_state_(new http2::http2_state(isolate_, http2_state_arrays)),
      created_from_snapshot_(true),
      as_external_(isolate_, as_external) {
  // We'll be creating new objects so make sure we've entered the context.
  v8::HandleScope handle_scope(isolate());
  v8::Context::Scope context_scope(context);

  CHECK_EQ(as_external->Value(), this);

  // FIXME
  AssignToContext(context, ContextInfo(""));

  destroy_async_id_list_.reserve(512);
  performance_state_ = Calloc<performance::performance_state>(1);
  performance_state_->milestones[
      performance::NODE_PERFORMANCE_MILESTONE_ENVIRONMENT] =
          PERFORMANCE_NOW();
  performance_state_->milestones[
    performance::NODE_PERFORMANCE_MILESTONE_NODE_START] =
        performance::performance_node_start;
  performance_state_->milestones[
    performance::NODE_PERFORMANCE_MILESTONE_V8_START] =
        performance::performance_v8_start;

#define V(PropertyName, TypeName)                                             \
  PropertyName##_.Reset(isolate(),                                            \
      context->GetDataFromSnapshotOnce<TypeName>(                             \
        ContextDepentSnapshotIndex::k##PropertyName).ToLocalChecked());
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_CONTEXT(V)
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_CONSOLE(V)
#undef V
  CHECK(context_.Get(isolate()) == context);


  fs_stats_field_array_ =
    reinterpret_cast<double*>((fs_stats_field_ab_.Get(isolate_))->GetContents().Data());

  ReSetupGlobalConsole(this);
}

int CreateSnapshot(uv_loop_t* event_loop,
                   int argc, const char* const* argv,
                   int exec_argc, const char* const* exec_argv) {
  intptr_t env_addr = reinterpret_cast<intptr_t> (new uint8_t[sizeof(Environment)]);
  intptr_t* references = SetupExternalReferences(env_addr);

  SnapshotCreator creator(references);
  ArrayBufferAllocator allocator;

  Isolate* const isolate = creator.GetIsolate();

  std::vector<AsyncWrap*> known_ptrs;
  Environment* env;
  {
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    creator.SetDefaultContext(context);
  }

  {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    IsolateData isolate_data(
        isolate,
        event_loop,
        nullptr,
        allocator.zero_fill_field());

    Local<Context> context = NewContext(isolate);
    Context::Scope context_scope(context);
    env = new (reinterpret_cast<void*>(env_addr)) Environment(&isolate_data, context);
    env->Start(argc, argv, exec_argc, exec_argv, false);

    {
      Environment::AsyncCallbackScope callback_scope(env);
      env->async_hooks()->push_async_ids(1, 0);
      LoadEnvironment(env);
      env->async_hooks()->pop_async_id(1);
    }

    CHECK_EQ(false, uv_loop_alive(env->event_loop()));

    {
      SetupGlobalConsole(env, &known_ptrs);
      AddObjectsToSnapshot(&creator, env);
    }

    creator.AddContext(context,
        v8::SerializeInternalFieldsCallback(SerializeInternalFields, &known_ptrs));

#if defined(LEAK_SANITIZER)
    __lsan_do_leak_check();
#endif
  }

  v8::StartupData startup = creator.CreateBlob(SnapshotCreator::FunctionCodeHandling::kClear);
  {
    SnapshotWriter writer;
    writer.SetStartupBlobFile("snapshot_blob.bin");
    writer.WriteSnapshot(startup);
  }
  delete startup.data;

  RestoreGlobalConsole(env, &known_ptrs);
  delete env;

  return 0;
}

void* SetupCreateParams(Isolate::CreateParams* params) {
  const char* filename = "snapshot_blob.bin";
  int fd = open(filename, O_RDONLY);
  struct stat s;
  fstat(fd, &s);

  char* data = reinterpret_cast<char*>(mmap(nullptr, static_cast<size_t>(s.st_size), PROT_READ, MAP_PRIVATE, fd, 0));

  v8::StartupData* blob = new v8::StartupData;
  blob->data = const_cast<const char*>(data);
  blob->raw_size =static_cast<int>(s.st_size);
  params->snapshot_blob = blob;

  void* env_addr = new uint8_t[sizeof(Environment)];
  intptr_t* references = SetupExternalReferences(reinterpret_cast<intptr_t>(env_addr));
  params->external_references = references;
  return env_addr;
}

namespace {

void DerializeInternalFields(Local<Object> holder, int index,
                                       v8::StartupData payload, void* data) {
}

IsolateData* NewIsolateDataFromSnapshot(Isolate* isolate, uv_loop_t* event_loop,
                                        MultiIsolatePlatform* platform,
                                        uint32_t* zero_fill_field) {
  IsolateData* isolate_data = new IsolateData(isolate, event_loop,
                                              platform, zero_fill_field,
                                              true);

  return isolate_data;
}

void SetupEnvTypedArrays(Local<Context> context,
                         std::vector<JSTypedArrays>* typed_arrays) {
  size_t typed_array_idx = ContextDepentSnapshotIndex::kTypedArrayStart;

  {
    Local<Float64Array> async_ids_stack =
      context->GetDataFromSnapshotOnce<Float64Array>(typed_array_idx++)
      .ToLocalChecked();

    JSTypedArrays js_array(async_ids_stack);
    typed_arrays->push_back(js_array);
  }

  {
    Local<Uint32Array> async_fields =
      context->GetDataFromSnapshotOnce<Uint32Array>(typed_array_idx++)
      .ToLocalChecked();

    JSTypedArrays js_array(async_fields);
    typed_arrays->push_back(js_array);
  }

  {
    Local<Float64Array> async_id_fields =
      context->GetDataFromSnapshotOnce<Float64Array>(typed_array_idx++)
      .ToLocalChecked();
    JSTypedArrays js_array(async_id_fields);
    typed_arrays->push_back(js_array);
  }

  {
    Local<Uint32Array> immediate_info_js_array =
      context->GetDataFromSnapshotOnce<Uint32Array>(typed_array_idx++)
      .ToLocalChecked();

    JSTypedArrays js_array(immediate_info_js_array);
    typed_arrays->push_back(js_array);
  }

  {
    Local<Uint8Array> tick_info_js_array =
      context->GetDataFromSnapshotOnce<Uint8Array>(typed_array_idx++)
      .ToLocalChecked();

    JSTypedArrays js_array(tick_info_js_array);
    typed_arrays->push_back(js_array);
  }

  {
    Local<Uint32Array> abort_on_uncaught_toggle_js_array =
      context->GetDataFromSnapshotOnce<Uint32Array>(typed_array_idx++)
      .ToLocalChecked();
    JSTypedArrays js_array(abort_on_uncaught_toggle_js_array);
    typed_arrays->push_back(js_array);
  }

  CHECK_EQ(ContextDepentSnapshotIndex::kHttp2StateStart, typed_array_idx);
}


void SetupHttp2StateArrays(Local<Context> context,
                           std::vector<JSTypedArrays>* http2_state_arrays) {
  size_t http2_state_start = ContextDepentSnapshotIndex::kHttp2StateStart;

#define ADD_HTTP2_STATE_FIELD(type, field)                                    \
  {                                                                           \
    Local<type> field =                                                       \
      context->GetDataFromSnapshotOnce<type>(http2_state_start++)             \
      .ToLocalChecked();                                                      \
    JSTypedArrays js_array(field);                                            \
    http2_state_arrays->push_back(js_array);                                  \
  }
  HTTP2_STATE_FIELDS(ADD_HTTP2_STATE_FIELD)
#undef ADD_HTTP2_STATE_FIELD
}

Environment* NewEnvironmentFromSnapshot(Isolate* isolate,
                                        IsolateData* isolate_data,
                                        Local<Context> context,
                                        void* env_addr) {
  HandleScope scope(isolate);

  std::vector<Local<String>> async_provider_strings;
  for (size_t i = 0; i < AsyncWrap::PROVIDERS_LENGTH; ++i) {
    async_provider_strings.push_back(isolate->GetDataFromSnapshotOnce<String>(
          ContextIndepentSnapshotIndex::kProviderStringStart + i)
          .ToLocalChecked());
  }

  Local<External> as_external =
    isolate->GetDataFromSnapshotOnce<External>(
        ContextIndepentSnapshotIndex::kAsExternal).ToLocalChecked();

  std::vector<JSTypedArrays> typed_arrays;
  SetupEnvTypedArrays(context, &typed_arrays);

  std::vector<JSTypedArrays> http2_state_arrays;
  SetupHttp2StateArrays(context, &http2_state_arrays);

  Environment* env =
    new (env_addr) Environment(isolate_data, context, as_external,
                               &typed_arrays,
                               &http2_state_arrays,
                               &async_provider_strings);
  return env;
}

}  // space

int StartFromSnapshot(Isolate* isolate, void* env_addr,
                      uv_loop_t* event_loop, MultiIsolatePlatform* platform,
                      uint32_t* zero_fill_field,
                      int argc, const char* const* argv,
                      int exec_argc, const char* const* exec_argv) {
  HandleScope handle_scope(isolate);

  Local<Context> context = Context::FromSnapshot(isolate, 0,
      v8::DeserializeInternalFieldsCallback(DerializeInternalFields))
    .ToLocalChecked();
  Context::Scope context_scope(context);

  IsolateData* isolate_data = NewIsolateDataFromSnapshot(
      isolate,
      event_loop,
      platform,
      zero_fill_field);

  Environment* env =
    NewEnvironmentFromSnapshot(isolate, isolate_data, context, env_addr);

  env->SetupUV(v8_is_profiling);
  SetupProcessObjectRuntimePart(env, argc, argv, exec_argc, exec_argv);
  LoadAsyncWrapperInfo(env);
  int exit_code = DoStart(isolate, env, argc, argv, exec_argc, exec_argv);

  delete env;
  delete isolate_data;

  return exit_code;
}

#undef HTTP2_STATE_FIELDS

}  // namespace node
