// Copyright 2011 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_HEAP_ALLOCATION_INFO_H_
#define V8_HEAP_ALLOCATION_INFO_H_

#include "include/v8.h"
#include "src/globals.h"
#include "src/utils.h"

namespace v8 {
namespace internal {
// -----------------------------------------------------------------------------
// A space has a circular list of pages. The next page can be accessed via
// Page::next_page() call.

// An abstraction of allocation and relocation pointers in a page-structured
// space.
class AllocationInfo {
 public:
  AllocationInfo() : top_(nullptr), limit_(nullptr) {}
  AllocationInfo(Address top, Address limit) : top_(top), limit_(limit) {}

  void Reset(Address top, Address limit) {
    set_top(top);
    set_limit(limit);
  }

  INLINE(void set_top(Address top)) {
    SLOW_DCHECK(top == NULL ||
                (reinterpret_cast<intptr_t>(top) & kHeapObjectTagMask) == 0);
    top_ = top;
  }

  INLINE(Address top()) const {
    SLOW_DCHECK(top_ == NULL ||
                (reinterpret_cast<intptr_t>(top_) & kHeapObjectTagMask) == 0);
    return top_;
  }

  Address* top_address() { return &top_; }

  INLINE(void set_limit(Address limit)) {
    limit_ = limit;
  }

  INLINE(Address limit()) const {
    return limit_;
  }

  Address* limit_address() { return &limit_; }

#ifdef DEBUG
  inline bool VerifyPagedAllocation();
#endif

 private:
  // Current allocation top.
  Address top_;
  // Current allocation limit.
  Address limit_;
};


}  // namespace internal
}  // namespace v8

#endif  // V8_HEAP_ALLOCATION_INFO_H_
