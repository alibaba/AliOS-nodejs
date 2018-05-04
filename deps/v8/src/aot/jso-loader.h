// Copyright 2017 the Alibaba YunOS project.  All rights reserved.

#ifndef V8_AOT_JSO_LOADER_H_
#define V8_AOT_JSO_LOADER_H_

#include "src/aot/jso-common.h"
#include "src/heap/mark-compact.h"

namespace v8 {
namespace internal {

class JsoLoader : public JsoCommonStatic {
 public:
  explicit JsoLoader(v8::Isolate* isolate);

  int Load(int fd, bool invertedReplaceFlag = false);

  MaybeLocal<UnboundScript> GetUnboundScript(unsigned i) const;

  Isolate* isolate() { return isolate_; }

 private:
  bool HasBeenLoaded(const JsoFileHeader* header);

  void MapAndLinkPages(int fd);

  void LinkInternalizedStrings(bool flipped);
  Local<Value> Throw(const char* message);

 private:

  Isolate* isolate_;
  const JsoFileHeader* header_;
  std::vector<Handle<SharedFunctionInfo>> unbound_scripts_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_AOT_JSO_GENERATOR_H_
