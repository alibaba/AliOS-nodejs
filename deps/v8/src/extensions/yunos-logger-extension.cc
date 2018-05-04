#include "src/extensions/yunos-logger-extension.h"
#include "src/api.h"
#include "src/handles.h"
#include "src/objects-inl.h"
#include "src/log.h"
#include "src/libsampler/sampler.h"
#include "include/v8.h"

namespace v8 {
namespace internal {

// supported flags
// static const char* kflags[13] = {
//   "log",
//   "log_all",
//   "log_api",
//   "log_code",
//   "log_gc",                     // impact gc, hydrogen
//   "log_handles",                // impact HandleEvent
//   "log_suspect",                // impact SuspectReadEvent
//   "log_regexp",                 // impact RegExpCompileEvent
//   "log_internal_timer_events",  // imply prof
//                                 // impact timer
//   "log_timer_events",           // imply log_internal_timer_events
//                                 // impact codegen
//   "logfile_per_isolate",
//   "prof_cpp",                   // new Profiler
//                                 // impact timer
//   "prof"                        // imply prof_cpp
//                                 // imply log_code
// };

Persistent<v8::String> YunOSLoggerExtension::kFlagsKey;
Persistent<v8::String> YunOSLoggerExtension::kFileName;
const char* const YunOSLoggerExtension::kSource =
    "native function YunOSLogger();";
bool YunOSLoggerExtension::kHasCreated = false;
bool YunOSLoggerExtension::kIsLogging = false;

v8::Local<v8::FunctionTemplate> YunOSLoggerExtension::GetNativeFunctionTemplate(
      v8::Isolate* isolate, v8::Local<v8::String> name) {
  Local<FunctionTemplate> logger_fun_template =
    FunctionTemplate::New(isolate, YunOSLoggerExtension::New);

  Local<v8::Signature> logger_signature =
    v8::Signature::New(isolate, logger_fun_template);

  logger_fun_template->SetClassName(
      v8::String::NewFromUtf8(isolate, "YunOSLogger", NewStringType::kNormal).ToLocalChecked());

  logger_fun_template->InstanceTemplate()->Set(
      isolate,
      "startLogging",
      FunctionTemplate::New(isolate, YunOSLoggerExtension::StartLogging, Local<Value>(), logger_signature));

  logger_fun_template->InstanceTemplate()->Set(
      isolate,
      "stopLogging",
      FunctionTemplate::New(isolate, YunOSLoggerExtension::StopLogging, Local<Value>(), logger_signature));

  logger_fun_template->ReadOnlyPrototype();
  return logger_fun_template;
}

static Local<Value> Throw(v8::Isolate* isolate, const char* message) {
  return isolate->ThrowException(v8::String::NewFromUtf8(isolate, message, NewStringType::kNormal).ToLocalChecked());
}

void YunOSLoggerExtension::New(const FunctionCallbackInfo<Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  if (kHasCreated) {
    Throw(isolate, "new YunOSLogger can only be called once");
    return;
  }

  kHasCreated = true;
  v8::HandleScope handle_scope(isolate);
  if (args.Length() < 2 || !args[0]->IsString()
                        || !args[1]->IsString()) {
    Throw(isolate, "arguments must be string");
    return;
  }

  if (!args.IsConstructCall()) {
    Throw(isolate, "YunOSLoggerExtension must be constructed with new");
    return;
  }

  // filename
  kFileName.Reset(isolate, Local<v8::String>::Cast(args[0]));

  // flags
  Local<Array> flags =
    Array::New(isolate, args.Length() - 1);

  for (uint32_t i = 0; i < flags->Length() ; ++i) {
    flags->Set(isolate->GetCurrentContext(), i, args[i+1]).FromJust();
  }

  kFlagsKey.Reset(isolate, v8::String::NewFromUtf8(isolate, "flags", NewStringType::kNormal).ToLocalChecked());
  args.Holder()->Set(isolate->GetCurrentContext(), Local<v8::String>::New(isolate, kFlagsKey), flags).FromJust();
}


static void SetFlags(const FunctionCallbackInfo<Value>& args, Local<v8::String> key) {
  v8::Isolate* isolate = args.GetIsolate();
  Local<Array> flags = Local<Array>::Cast(args.Holder()->Get(isolate->GetCurrentContext(), key).ToLocalChecked());

  for (uint32_t i = 0; i < flags->Length(); ++i) {
    Local<v8::String> flag = Local<v8::String>::Cast(flags->Get(isolate->GetCurrentContext(), i).ToLocalChecked());
    int length = flag->Utf8Length() + 3; // '-', '-', and null terminator
    char* buffer = static_cast<char*> (malloc(length));
    buffer[0] = '-';
    buffer[1] = '-';

    flag->WriteUtf8(buffer+2);
    FlagList::SetFlagsFromString(buffer, length);
    free(buffer);
  }
}

void YunOSLoggerExtension::StartLogging(const FunctionCallbackInfo<Value>& args) {
  if (kIsLogging) {
    PrintF(stderr, "WARNING: YunOSLogger is already logging\n");
    return;
  }
  kIsLogging = true;

  v8::Isolate* isolate = args.GetIsolate();
  bool last_log_gc = FLAG_log_gc;
  bool last_log_timer_events = FLAG_log_timer_events;
  bool last_perf_basic_prof = FLAG_perf_basic_prof;

  SetFlags(args, Local<v8::String>::New(isolate, kFlagsKey));
  if (FLAG_log_gc && !last_log_gc) {
    const char* f = "log_gc";
    PrintF(stderr, "WARNING: to enable %s, v8 must start with --%s\n", f, f);
    FLAG_log_gc = last_log_gc;
  }

  if (FLAG_log_timer_events && !last_log_timer_events) {
    const char* f= "log_timer_events";
    PrintF(stderr, "WARNING: to enable %s, v8 must start with --%s\n", f, f);
    FLAG_log_timer_events = last_log_timer_events;
  }

  if (FLAG_perf_basic_prof && FLAG_compact_code_space) {
    const char* f = "perf_basic_prof";
    PrintF(stderr, "WARNING: to enable %s, v8 must start with --no_compact_code_space\n", f);
    FLAG_perf_basic_prof = last_perf_basic_prof;
  }

  Isolate* iisolate = reinterpret_cast<Isolate*> (isolate);
  Logger *logger = iisolate->logger();

  // processing implies
  if (FLAG_log_timer_events) {
    FLAG_log_internal_timer_events = true;
  }

  if (FLAG_log_internal_timer_events) {
    FLAG_prof = true;
  }

  if (FLAG_prof) {
    FLAG_prof_cpp = true;
  }

  Local<v8::String> filename = Local<v8::String>::New(isolate, kFileName);
  int length = filename->Utf8Length() + 1; // null terminator
  char* buffer = static_cast<char*> (malloc(length));
  filename->WriteUtf8(buffer);

  logger->SetUp(iisolate, buffer);
  free(buffer);

  {
    HandleScope scope(iisolate);
    LOG_CODE_EVENT(iisolate, LogCodeObjects());
    LOG_CODE_EVENT(iisolate, LogCompiledFunctions());
  }
}

void YunOSLoggerExtension::StopLogging(const FunctionCallbackInfo<Value>& args) {
  if (!kIsLogging) {
    PrintF(stderr, "WARNING: YunOSLogger is not logging\n");
    return;
  }
  kIsLogging = false;

  v8::Isolate* isolate = args.GetIsolate();
  Isolate* iisolate = reinterpret_cast<Isolate*> (isolate);
  Logger *logger = iisolate->logger();

  // We must stop the logger before we tear down other components.
  v8::sampler::Sampler* sampler = logger->sampler();
  if (sampler && sampler->IsActive()) sampler->Stop();

  logger->TearDown();
}


}  // namespace internal
}  // namespace v8
