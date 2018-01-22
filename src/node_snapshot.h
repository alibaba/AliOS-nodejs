#ifndef SRC_NODE_SNAPSHOT_H_
#define SRC_NODE_SNAPSHOT_H_

#if defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#include "node_internals.h"
#include "node_debug_options.h"
#include "node.h"
#include "v8.h"

namespace node {

#define ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_JSARRAY(V) \
  V(ASYNC_HOOKS_ASYNC_IDS_STACK, v8::Float64Array) \
  V(ASYNC_HOOKS_FIELDS, v8::Uint32Array) \
  V(ASYNC_HOOKS_ASYNC_ID_FIELDS, v8::Float64Array) \
  V(TICK_INFO_FIELDS, v8::Uint8Array) \
  V(IMMEDIATE_INFO_FIELDS, v8::Uint8Array) \
  V(SHOULD_ABORT_ON_UNCAUGHT_TOGGLE, v8::Uint32Array) \
  V(HTTP2_STATE_ROOT_BUFFER, v8::Uint8Array) \
  V(HTTP2_STATE_SESSION_STATE_BUFFER, v8::Float64Array) \
  V(HTTP2_STATE_STREAM_STATE_BUFFER, v8::Float64Array) \
  V(HTTP2_STATE_STREAM_STATS_BUFFER, v8::Float64Array) \
  V(HTTP2_STATE_SESSION_STATS_BUFFER, v8::Float64Array) \
  V(HTTP2_STATE_PADDING_BUFFER, v8::Uint32Array) \
  V(HTTP2_STATE_OPTIONS_BUFFER, v8::Uint32Array) \
  V(HTTP2_STATE_SETTINGS_BUFFER, v8::Uint32Array)

enum ContextSnapshotIndex : size_t {
#define V(PropertyName, TypeName)                                             \
  k##PropertyName,
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_CONTEXT(V)
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_CONSOLE(V)
  ENVIRONMENT_STRONG_PERSISTENT_PROPERTIES_JSARRAY(V)
#undef V
};

extern bool v8_is_profiling;

int CreateSnapshot(uv_loop_t* event_loop,
                   int argc, const char* const* argv,
                   int exec_argc, const char* const* exec_argv);

int StartFromSnapshot(v8::Isolate* isolate, void* env_addr,
                      uv_loop_t* event_loop, MultiIsolatePlatform* platform,
                      uint32_t* zero_fill_field,
                      int argc, const char* const* argv,
                      int exec_argc, const char* const* exec_argv);

int DoStart(v8::Isolate* isolate, Environment* env,
            int argc, const char* const* argv,
            int exec_argc, const char* const* exec_argv);

void* SetupCreateParams(v8::Isolate::CreateParams* params);

void SetupProcessObjectStaticPart(Environment* env,
                                  int argc,
                                  const char* const* argv,
                                  int exec_argc,
                                  const char* const* exec_argv);

void SetupProcessObjectRuntimePart(Environment* env,
                                   int argc,
                                   const char* const* argv,
                                   int exec_argc,
                                  const char* const* exec_argv);

}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#endif // SRC_NODE_SNAPSHOT_H_
