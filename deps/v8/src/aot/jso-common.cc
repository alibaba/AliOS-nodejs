// Copyright 2017 the Alibaba YunOS project.  All rights reserved.

#include "src/aot/jso-common.h"

#include "src/base/platform/platform.h"
#include "src/isolate.h"

namespace v8 {
namespace internal {

size_t JsoFileHeader::HeaderSize() const {
  return sizeof(*this) + (num_internalized_strings
                          + num_unbound_scripts
                          + num_normal_object_pages
                          + num_large_object_pages) * sizeof(intptr_t);
}

size_t JsoFileHeader::MemoryChunkStartOffset() const {
  return RoundUp(HeaderSize(), base::OS::CommitPageSize());
}

size_t JsoFileHeader::NormalObjectPagesOffset() const {
  return MemoryChunkStartOffset();
}

size_t JsoFileHeader::LargeObjectPagesOffset() const {
  return NormalObjectPagesOffset() + num_normal_object_pages * Page::kPageSize;
}

SharedFunctionInfo* const* JsoFileHeader::unbound_scripts() const {
  return (SharedFunctionInfo* const*)((uint8_t*)this + sizeof(*this));
}

String* const* JsoFileHeader::internalized_strings() const {
  return (String* const*)((uint8_t*)unbound_scripts()
                          + num_unbound_scripts * sizeof(intptr_t));
}

Page* const* JsoFileHeader::normal_object_pages() const {
  return (Page* const*)((uint8_t*)internalized_strings()
                        + num_internalized_strings * sizeof(intptr_t));
}

LargePage* const* JsoFileHeader::large_object_pages() const {
  return (LargePage* const*)((uint8_t*)normal_object_pages()
                             + num_normal_object_pages * sizeof(intptr_t));
}

bool JsoFileHeader::IsChunkInThisFile(const MemoryChunk* chunk) const {
  auto it = (MemoryChunk* const*)normal_object_pages();
  auto end = it + num_normal_object_pages + num_large_object_pages;
  for (; it != end; it++)
    if (chunk == *it)
      return true;
  return false;
}

intptr_t JsoCommonStatic::ComputeHeapChecksum(Isolate* isolate) {
  // TODO(aot): involve more root and partial_snapshot_cache objects.
  const auto ptr = isolate->heap()->root(Heap::kSharedFunctionInfoMapRootIndex);
  return reinterpret_cast<intptr_t>(ptr);
}

}  // namespace internal
}  // namespace v8
