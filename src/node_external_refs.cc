#if 0
#if !(NODE_USE_SNAPSHOT)
#regor __FILE__ is used only for the snapshot build.
#endif
#endif

#include "v8.h"  // NOLINT(build/include_order)
#include "node.h"
#include "env.h"
#include "node_external_refs.h"

#if 0
#include "node_buffer.h"
#include "node_constants.h"
#include "node_file.h"
#include "node_http_parser.h"
#include "node_javascript.h"
#include "node_version.h"
#include "node_internals.h"
#include "node_revert.h"
#include "node_thread_worker.h"
#endif

namespace node {

using v8::FunctionCallbackInfo;
using v8::Value;

#if 0
namespace util {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}
#endif
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

#if 0
namespace os {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}

namespace url {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}

namespace Wfork {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}

namespace worker {
  extern void RegisterExternalReferences(ExternalReferenceRegister* reg);
}

extern void ContextifyRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void TimerRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void FileRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void FSEventRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void YunosLogRegisterExternalReferences(ExternalReferenceRegister* reg);
#endif
extern void NodeRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void AsyncWrapRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void ContextifyRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void FSRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void FSEventWrapRegisterExternalReferences(ExternalReferenceRegister* reg);
extern void UVRegisterExternalReferences(ExternalReferenceRegister* reg);

void InitExternalReferences(ExternalReferenceRegister* reg, uint8_t* env_addr) {
  reg->add(reinterpret_cast<intptr_t> (env_addr));

  NodeRegisterExternalReferences(reg);
  AsyncWrapRegisterExternalReferences(reg);
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
#if 0

  util::RegisterExternalReferences(reg);
  Wfork::RegisterExternalReferences(reg);
  worker::RegisterExternalReferences(reg);
  ContextifyRegisterExternalReferences(reg);
  TimerRegisterExternalReferences(reg);
  FileRegisterExternalReferences(reg);
  FSEventRegisterExternalReferences(reg);
  YunosLogRegisterExternalReferences(reg);

#endif
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
