// Copyright 2017 the Alibaba YunOS project.  All rights reserved.

#ifndef V8_AOT_JSO_RANGE_H_
#define V8_AOT_JSO_RANGE_H_

#include "src/heap/heap.h"

namespace v8 {
namespace internal {

struct JsoFileHeader;

class JsoRange {
 public:
  static bool InitializeJsoRange(uintptr_t raw_addr, size_t size);

  static void TakeControlOfCodeRange(base::VirtualMemory* to_range);

 public:
  explicit JsoRange(Isolate* isolate);
  ~JsoRange() { TearDown(); }

  void SetUp();

  bool IsValid() { return jso_range_.IsReserved(); }

  bool Contains(Address address);

  bool ShouldAllocateInJsoRange(Space* owner);

  // Allocates a chunk of memory from the large-object portion of
  // the jso range.
  MUST_USE_RESULT Address AllocateRawMemory(const size_t requested_size,
                                            const size_t commit_size,
                                            const char* space_name,
                                            Executability executable,
                                            size_t* allocated);

  bool CommitRawMemory(Address start, size_t length, Executability executable);

  bool UncommitRawMemory(Address start, size_t length);

  void FreeRawMemory(Address buf, size_t length);

  void AddLoadedBlock(Address address, size_t length);

  void AddLoadedJsoFileHeader(const JsoFileHeader* header);

  template<typename Callback>
  const JsoFileHeader* FindJsoFileHeader(Callback callback) const {
    for (auto header : jso_file_headers_)
      if (callback(header))
        return header;
    return nullptr;
  }

 private:
  struct FreeBlock {
    FreeBlock(Address start_arg = 0, size_t size_arg = 0)
        : start(start_arg), size(size_arg) {
    }

    Address start;
    size_t size;
  };

  // Frees the range of virtual memory, and frees the data structures used to
  // manage it.
  void TearDown();

  // Finds a block on the allocation list that contains at least the
  // requested amount of memory.  If none is found, sorts and merges
  // the existing free memory blocks, and searches again.
  // If none can be found, returns false.
  bool GetNextAllocationBlock(size_t requested);

  // Sort and merge the free blocks on the free list and the allocation list.
  void MergeFreeListToAllocationList();

  // Compares the start addresses of two free blocks.
  static int CompareFreeBlockAddress(const FreeBlock* left,
                                     const FreeBlock* right);

  void MergeLoadedList();

  bool ReserveBlock(const size_t requested_size, FreeBlock* block);

  void ReleaseBlock(const FreeBlock* block);

  static void TakeControlOfJsoRange(base::VirtualMemory* to_range);

 private:
  Isolate* isolate_;

  // The reserved range of virtual memory that all jso chunks are put in.
  base::VirtualMemory jso_range_;

  // The global mutex guards free_list_ and allocation_list_ as GC threads may
  // access both lists concurrently to the main thread.
  base::Mutex jso_range_mutex_;

  // Freed blocks of memory are added to the free list.  When the allocation
  // list is exhausted, the free list is sorted and merged to make the new
  // allocation list.
  List<FreeBlock> free_list_;

  // Memory is allocated from the free blocks on the allocation list.
  // The block at current_allocation_block_index_ is the current block.
  List<FreeBlock> allocation_list_;
  int current_allocation_block_index_;

  // Blocks loaded by JsoLoader.
  List<FreeBlock> loaded_list_;

  // Headers of loaded jso files.
  List<const JsoFileHeader*> jso_file_headers_;

  static base::VirtualMemory jso_range_reserved_;
  static base::VirtualMemory code_range_reserved_;

  DISALLOW_COPY_AND_ASSIGN(JsoRange);
};

}  // namespace internal
}  // namespace v8

#endif  // V8_AOT_JSO_RANGE_H_
