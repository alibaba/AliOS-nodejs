// Copyright 2017 the Alibaba YunOS project.  All rights reserved.

#include "src/aot/jso-range.h"

#include <sys/mman.h>

#include "src/heap/spaces.h"
#include "src/isolate.h"
#include "src/list-inl.h"

namespace v8 {
namespace internal {

const uintptr_t kJsoAddress = 0x20000000;
const size_t kJsoRangeSize = 512 * MB * (kPointerSize / 4);

JsoRange::JsoRange(Isolate* isolate)
    : isolate_(isolate),
      free_list_(0),
      allocation_list_(0),
      current_allocation_block_index_(0) {}

void JsoRange::SetUp() {
  CHECK(!jso_range_.IsReserved());
  TakeControlOfJsoRange(&jso_range_);
  CHECK(jso_range_.IsReserved());
  Address base = reinterpret_cast<Address>(jso_range_.address());
  Address aligned_base = RoundUp(base, MemoryChunk::kAlignment);
  size_t size = jso_range_.size() - (aligned_base - base);
  allocation_list_.Add(FreeBlock(aligned_base, size));
  current_allocation_block_index_ = 0;
}

bool JsoRange::Contains(Address address) {
  if (!jso_range_.IsReserved())
    return false;
  const auto start = static_cast<Address>(jso_range_.address());
  return start <= address && address < start + jso_range_.size();
}

bool JsoRange::ShouldAllocateInJsoRange(Space* owner) {
  // The whole snapsnot is allocated in JsoRange; when doing AOT compilation,
  // large objects are always allocated in JsoRange, other objects are allocated
  // in JsoRange only when migrating objects.
  // JsoGenerator::MarkingVisitor::IsLoadedJso() depends on this.
  return !isolate_->heap()->deserialization_complete() ||
      (isolate_->is_doing_aot_compilation() &&
       (owner->identity() == LO_SPACE ||
        isolate_->heap()->gc_state() == Heap::AOT_MARK_MIGRATE));
}

Address JsoRange::AllocateRawMemory(const size_t requested_size,
                                    const size_t commit_size,
                                    const char* space_name,
                                    Executability executable,
                                    size_t* allocated) {
  if (loaded_list_.length() > 0)
    MergeLoadedList();
  FreeBlock current;
  if (!ReserveBlock(requested_size, &current)) {
    *allocated = 0;
    return NULL;
  }
  *allocated = current.size;
  DCHECK(IsAddressAligned(current.start, MemoryChunk::kAlignment));
  bool result;
  if (executable == EXECUTABLE) {
    CHECK_LE(commit_size,
             requested_size - 2 * MemoryAllocator::CodePageGuardSize());
    result = isolate_->heap()->memory_allocator()->CommitExecutableMemory(
        &jso_range_, current.start, commit_size, current.size, space_name);
  } else {
    result = isolate_->heap()->memory_allocator()->CommitMemory(
        current.start, current.size, executable, space_name);
  }
  if (!result) {
    ReleaseBlock(&current);
    *allocated = 0;
    return NULL;
  }
  return current.start;
}


bool JsoRange::CommitRawMemory(Address start, size_t length,
                               Executability executable) {
  return isolate_->heap()->memory_allocator()->CommitMemory(start, length,
                                                            executable,
                                                            "JsoRange::CommitRawMemory");
}

bool JsoRange::UncommitRawMemory(Address start, size_t length) {
  return jso_range_.Uncommit(start, length);
}

void JsoRange::FreeRawMemory(Address address, size_t length) {
  DCHECK(IsAddressAligned(address, MemoryChunk::kAlignment));
  base::LockGuard<base::Mutex> guard(&jso_range_mutex_);
  free_list_.Add(FreeBlock(address, length));
  jso_range_.Uncommit(address, length);
}

void JsoRange::AddLoadedBlock(Address address, size_t length) {
  DCHECK(IsAddressAligned(address, MemoryChunk::kAlignment));
  base::LockGuard<base::Mutex> guard(&jso_range_mutex_);
  loaded_list_.Add(FreeBlock(address, length));
}

void JsoRange::AddLoadedJsoFileHeader(const JsoFileHeader* header) {
  jso_file_headers_.Add(header);
}

void JsoRange::TearDown() {
  base::LockGuard<base::Mutex> guard(&jso_range_mutex_);
  free_list_.Free();
  allocation_list_.Free();
}

bool JsoRange::GetNextAllocationBlock(size_t requested) {
  for (current_allocation_block_index_++;
       current_allocation_block_index_ < allocation_list_.length();
       current_allocation_block_index_++) {
    if (requested <= allocation_list_[current_allocation_block_index_].size) {
      return true;  // Found a large enough allocation block.
    }
  }
  MergeFreeListToAllocationList();
  for (current_allocation_block_index_ = 0;
       current_allocation_block_index_ < allocation_list_.length();
       current_allocation_block_index_++) {
    if (requested <= allocation_list_[current_allocation_block_index_].size) {
      return true;  // Found a large enough allocation block.
    }
  }
  current_allocation_block_index_ = 0;
  // Jso range is full or too fragmented.
  return false;
}

void JsoRange::MergeFreeListToAllocationList() {
  free_list_.AddAll(allocation_list_);
  allocation_list_.Clear();
  free_list_.Sort(&CompareFreeBlockAddress);
  for (int i = 0; i < free_list_.length();) {
    FreeBlock merged = free_list_[i];
    i++;
    // Add adjacent free blocks to the current merged block.
    while (i < free_list_.length() &&
           free_list_[i].start == merged.start + merged.size) {
      merged.size += free_list_[i].size;
      i++;
    }
    if (merged.size > 0) {
      allocation_list_.Add(merged);
    }
  }
  free_list_.Clear();
}

int JsoRange::CompareFreeBlockAddress(const FreeBlock* left,
                                      const FreeBlock* right) {
  // The entire point of JsoRange is that the difference between two
  // addresses in the range can be represented as a signed 32-bit int,
  // so the cast is semantically correct.
  return static_cast<int>(left->start - right->start);
}

void JsoRange::MergeLoadedList() {
  if (free_list_.length() > 0)
    MergeFreeListToAllocationList();
  loaded_list_.Sort(&CompareFreeBlockAddress);
  for (int i = 0, j = 0; i < allocation_list_.length(); i++) {
    auto &block = allocation_list_[i];
    const auto block_end = block.start + block.size;
    if (loaded_list_[j].start >= block_end)
      continue;
    DCHECK(block.start <= loaded_list_[j].start);
    block.size = loaded_list_[j].start - block.start;
    for (Address end = nullptr; end != block_end;) {
      const auto start = loaded_list_[j].start + loaded_list_[j].size;
      j++;
      end = (j < loaded_list_.length()) && (loaded_list_[j].start < block_end)
          ? loaded_list_[j].start : block_end;
      const auto free_size = end - start;
      if (free_size > 0)
        free_list_.Add(FreeBlock(start, free_size));
    }
  }
  loaded_list_.Clear();
}

bool JsoRange::ReserveBlock(const size_t requested_size, FreeBlock* block) {
  base::LockGuard<base::Mutex> guard(&jso_range_mutex_);
  DCHECK(allocation_list_.length() == 0 ||
         current_allocation_block_index_ < allocation_list_.length());
  if (allocation_list_.length() == 0 ||
      requested_size > allocation_list_[current_allocation_block_index_].size) {
    // Find an allocation block large enough.
    if (!GetNextAllocationBlock(requested_size)) return false;
  }
  // Commit the requested memory at the start of the current allocation block.
  size_t aligned_requested = RoundUp(requested_size, MemoryChunk::kAlignment);
  *block = allocation_list_[current_allocation_block_index_];
  // Don't leave a small free block, useless for a large object or chunk.
  if (aligned_requested < (block->size - Page::kPageSize)) {
    block->size = aligned_requested;
  }
  DCHECK(IsAddressAligned(block->start, MemoryChunk::kAlignment));
  allocation_list_[current_allocation_block_index_].start += block->size;
  allocation_list_[current_allocation_block_index_].size -= block->size;
  return true;
}

void JsoRange::ReleaseBlock(const FreeBlock* block) {
  base::LockGuard<base::Mutex> guard(&jso_range_mutex_);
  free_list_.Add(*block);
}

base::VirtualMemory JsoRange::jso_range_reserved_;
base::VirtualMemory JsoRange::code_range_reserved_;

bool JsoRange::InitializeJsoRange(uintptr_t raw_addr, size_t size) {
  // It can only be initialized once.
  CHECK(!jso_range_reserved_.IsReserved());
  if (raw_addr == 0)
    raw_addr = kJsoAddress;
  if (size == 0) {
    size = kJsoRangeSize;
  } else {
    size = size * MB;
    if (size > 2 * kJsoRangeSize) {
      size = 2 * kJsoRangeSize;
    }
  }
  const size_t code_size = kRequiresCodeRange ? kMaximalCodeRangeSize : 0;
  const size_t total_size = size + code_size;
  CHECK((raw_addr % MemoryChunk::kAlignment) == 0 &&
        (total_size % MemoryChunk::kAlignment) == 0);
  void* address =
      mmap(reinterpret_cast<void*>(raw_addr), total_size, PROT_NONE,
           MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_FIXED, -1, 0);
  if (address == MAP_FAILED)
    return false;
  base::VirtualMemory jso_range(address, size);
  jso_range_reserved_.TakeControl(&jso_range);
  if (code_size) {
    base::VirtualMemory code_range(reinterpret_cast<void*>(raw_addr + size), code_size);
    code_range_reserved_.TakeControl(&code_range);
  }
  return jso_range_reserved_.IsReserved();
}

void JsoRange::TakeControlOfJsoRange(base::VirtualMemory* to_range) {
  CHECK(jso_range_reserved_.IsReserved());
  to_range->TakeControl(&jso_range_reserved_);
}

void JsoRange::TakeControlOfCodeRange(base::VirtualMemory* to_range) {
  CHECK(code_range_reserved_.IsReserved());
  to_range->TakeControl(&code_range_reserved_);
}

}  // namespace internal
}  // namespace v8
