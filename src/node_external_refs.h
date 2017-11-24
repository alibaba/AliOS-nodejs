#ifndef SRC_NODE_EXTERNAL_REFS_H_
#define SRC_NODE_EXTERNAL_REFS_H_

#if 0
#if !(NODE_USE_SNAPSHOT)
#error __FILE__ is used only for the snapshot build.
#endif
#endif

#include <vector>

#include "v8.h"

namespace node {
using v8::Isolate;

class ExternalReferenceRegister {
 public:
  ExternalReferenceRegister() : frozen_(false) {}

  void add(v8::FunctionCallback callback) {
    add(reinterpret_cast<intptr_t> (callback));
  }
  void add(v8::NamedPropertyGetterCallback callback) {
    add(reinterpret_cast<intptr_t> (callback));
  }

  void add(intptr_t addr) {
    // CHECK(!frozen_);
    list_.push_back(addr);
  }

  intptr_t* external_references(size_t* size = nullptr) {
    frozen_ = true;
    // freed by caller
    intptr_t* external_references = new intptr_t[list_.size()];
    for (size_t i = 0; i < list_.size(); ++i) {
      external_references[i] = list_[i];
    }

    if (size) {
      // strip null sentinel
      *size = list_.size() - 1;
    }
    return external_references;
  }

 private:
  std::vector<intptr_t> list_;
  bool frozen_;
};

void InitExternalReferences(ExternalReferenceRegister* reg, uint8_t* env_addr);
uint8_t *SetupExternalReferences(Isolate::CreateParams* params);

}
#endif
