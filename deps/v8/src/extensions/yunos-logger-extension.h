#ifndef V8_YUNOS_LOGGER_H_
#define V8_YUNOS_LOGGER_H_

#include "include/v8.h"


namespace v8 {
namespace internal {

class YunOSLoggerExtension : public v8::Extension {
 public:
  explicit YunOSLoggerExtension ()
      : v8::Extension("v8/YunOSLogger", kSource) {}

  virtual v8::Local<v8::FunctionTemplate> GetNativeFunctionTemplate(
      v8::Isolate* isolate, v8::Local<v8::String> name) override;

 private:
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void StartLogging(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void StopLogging(const v8::FunctionCallbackInfo<v8::Value>& args);
  static Persistent<v8::String> kFlagsKey;
  static Persistent<v8::String> kFileName;
  static const char* const kSource;
  static bool kHasCreated;
  static bool kIsLogging;
};


}  // namespace internal
}  // namespace v8

#endif  // V8_YUNOS_LOGGER_H_
