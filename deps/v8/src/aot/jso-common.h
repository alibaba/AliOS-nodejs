// Copyright 2017 the Alibaba YunOS project.  All rights reserved.

#ifndef V8_AOT_JSO_COMMON_H_
#define V8_AOT_JSO_COMMON_H_

#include "src/heap/spaces.h"

namespace v8 {
namespace internal {

/**
 * JS object file format:
 *  Header
 *   JsoFileHeader header
 *   intptr_t pointers_of_unbound_scripts[header.num_unbound_scripts]
 *   intptr_t pointers_of_internalized_strings[header.num_internalized_strings]
 *   intptr_t addresses_of_normal_object_pages[header.num_normal_object_pages]
 *   intptr_t addresses_of_large_object_pages[header.num_large_object_pages]
 *  normal object pages
 *  large object pages
 */
struct JsoFileHeader {
  const uint8_t magic_number[4] = {'J', 'S', 'O', '\0'};
  uint32_t num_unbound_scripts = 0;
  uint32_t num_internalized_strings = 0;
  uint32_t num_normal_object_pages = 0;
  uint32_t num_large_object_pages = 0;
  intptr_t heap_checksum = 0;

 public:
  size_t HeaderSize() const;

  size_t MemoryChunkStartOffset() const;
  size_t NormalObjectPagesOffset() const;
  size_t LargeObjectPagesOffset() const;

  SharedFunctionInfo* const* unbound_scripts() const;
  String* const* internalized_strings() const;
  Page* const* normal_object_pages() const;
  LargePage* const* large_object_pages() const;

  bool IsChunkInThisFile(const MemoryChunk* chunk) const;
};

class JsoCommonStatic {
 protected:
  static intptr_t ComputeHeapChecksum(Isolate* isolate);
};

}  // namespace internal
}  // namespace v8
#endif  // V8_AOT_JSO_COMMON_H_
