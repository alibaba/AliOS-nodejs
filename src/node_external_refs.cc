#include "v8.h"  // NOLINT(build/include_order)
#include "node.h"
#include "env.h"
#include "node_external_refs.h"
#include "module_wrap.h"
#include "node_javascript.h"

namespace node {

using v8::FunctionCallbackInfo;
using v8::Value;

namespace Buffer {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}
namespace cares_wrap {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}
namespace performance {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}
namespace i18n {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}
namespace util {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}
namespace url {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}
namespace inspector {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}

extern void NodeRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void ContextifyRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void FSRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void FSEventWrapRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void UVRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void HandleWrapRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void AsyncWrapRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void StreamRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void TTYRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void SignalRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void TimerWrapRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void TCPRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void PipeRegisterExternalReferences(ExternalReferenceRegister* reg);

void InitExternalReferences(ExternalReferenceRegister* reg, uint8_t* env_addr) {
  reg->add(reinterpret_cast<intptr_t> (env_addr));

  NodeRegisterExternalReferences(reg);
  Buffer::RegisterExternalReferences(reg);
  cares_wrap::RegisterExternalReferences(reg);
  ContextifyRegisterExternalReferences(reg);
  FSRegisterExternalReferences(reg);
  FSEventWrapRegisterExternalReferences(reg);
  performance::RegisterExternalReferences(reg);
  i18n::RegisterExternalReferences(reg);
  util::RegisterExternalReferences(reg);
  UVRegisterExternalReferences(reg);
  url::RegisterExternalReferences(reg);
  inspector::RegisterExternalReferences(reg);
  loader::ModuleWrap::RegisterExternalReferences(reg);

  HandleWrapRegisterExternalReferences(reg);
  AsyncWrapRegisterExternalReferences(reg);
  StreamRegisterExternalReferences(reg);
  TTYRegisterExternalReferences(reg);
  SignalRegisterExternalReferences(reg);
  TimerWrapRegisterExternalReferences(reg);
  TCPRegisterExternalReferences(reg);
  PipeRegisterExternalReferences(reg);

  size_t length;
  v8::String::ExternalStringResourceBase** resources = NativeSourceResources(&length);
  for (size_t i = 0; i < length; ++i) {
    reg->add(reinterpret_cast<intptr_t> (resources[i]));
  }

  reg->add(NULL);
}

uint8_t* SetupExternalReferences(v8::Isolate::CreateParams* params) {
  uint8_t* env_addr = new uint8_t[sizeof(Environment)];
  ExternalReferenceRegister reg;
  InitExternalReferences(&reg, env_addr);

  params->external_references = reg.external_references();
  return env_addr;
}

} //  namespace node
