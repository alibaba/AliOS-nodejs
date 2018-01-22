#include "v8.h"

namespace node {
class Environment;

  size_t _templates_contextify(const v8::FunctionCallback**) { return 0; }
  size_t _templates_config(const v8::FunctionCallback**) { return 0; }
  size_t _templates_v8(const v8::FunctionCallback**) { return 0; }
  size_t _templates_uv(const v8::FunctionCallback**) { return 0; }
  size_t _templates_util(const v8::FunctionCallback**) { return 0; }
  size_t _templates_trace_events(const v8::FunctionCallback**) { return 0; }
  size_t _templates_timer_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_fs(const v8::FunctionCallback**) { return 0; }
  size_t _templates_fs_event_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_tty_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_cares_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_tcp_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_pipe_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_stream_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_signal_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_os(const v8::FunctionCallback**) { return 0; }
  size_t _templates_http2(const v8::FunctionCallback**) { return 0; }
  size_t _templates_http_parser(const v8::FunctionCallback**) { return 0; }
  size_t _templates_js_stream(const v8::FunctionCallback**) { return 0; }
  size_t _templates_module_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_process_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_serdes(const v8::FunctionCallback**) { return 0; }
  size_t _templates_spawn_sync(const v8::FunctionCallback**) { return 0; }
  size_t _templates_udp_wrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_zlib(const v8::FunctionCallback**) { return 0; }
  size_t _templates_handlewrap(const v8::FunctionCallback**) { return 0; }
  size_t _templates_statwatcher(const v8::FunctionCallback**) { return 0; }


  void ReConstructTTYWrap(node::Environment*, v8::Local<v8::Object>, int, bool) {}

  namespace cares_wrap {
    void ReConstructChannelWrap(node::Environment*, v8::Local<v8::Object>) {}
  }

}
