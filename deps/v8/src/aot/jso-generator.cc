// Copyright 2017 the Alibaba YunOS project.  All rights reserved.
#include "src/aot/jso-generator.h"

#include <set>
#include "src/address-map.h"
#include "src/objects-inl.h"
#include "src/heap/mark-compact-inl.h"

namespace v8 {
namespace internal {

class UnlinkWeakNextScope {
 public:
  explicit UnlinkWeakNextScope(HeapObject* object) : object_(nullptr) {
    if (object->IsWeakCell()) {
      object_ = object;
      next_ = WeakCell::cast(object)->next();
      WeakCell::cast(object)->clear_next(object->GetHeap()->the_hole_value());
    } else if (object->IsAllocationSite()) {
      object_ = object;
      next_ = AllocationSite::cast(object)->weak_next();
      AllocationSite::cast(object)->set_weak_next(
          object->GetHeap()->undefined_value());
    }
  }

  ~UnlinkWeakNextScope() {
    if (object_ != nullptr) {
      if (object_->IsWeakCell()) {
        WeakCell::cast(object_)->set_next(next_, UPDATE_WEAK_WRITE_BARRIER);
      } else {
        AllocationSite::cast(object_)->set_weak_next(next_,
                                                     UPDATE_WEAK_WRITE_BARRIER);
      }
    }
  }

 private:
  HeapObject* object_;
  Object* next_;
  DisallowHeapAllocation no_gc_;
};

class JsoGenerator::MarkingVisitor final : public ObjectVisitor {
 public:
  MarkingVisitor(Isolate* isolate)
      : marking_stack_(16),
        jso_range_(isolate->heap()->memory_allocator()->jso_range()),
        root_index_map_(isolate) {}

  void AddRoot(Object* script) {
    MarkObject(HeapObject::cast(script));
  }

  void TransitiveClosure() {
    while (!marking_stack_.is_empty()) {
      current_object_ = marking_stack_.RemoveLast();
      auto chunk = MemoryChunk::FromAddress(reinterpret_cast<Address>(current_object_));
      if (chunk->owner()->identity() != SHARED_LO_SPACE) {
        marked_pages_.insert(static_cast<Page*>(chunk));
      } else {
        marked_large_pages_.insert(static_cast<LargePage*>(chunk));
      }
      UnlinkWeakNextScope unlink_weak_next(current_object_);
      current_object_->IterateBody(this);
    }
  }

  template<typename Callback>
  void ForEachMarkedPage(Callback callback) {
    for (auto chunk : marked_pages_)
      callback(chunk);
  }

  template<typename Callback>
  void ForEachMarkedLargePage(Callback callback) {
    for (auto chunk : marked_large_pages_)
      callback(chunk);
  }

 private:
  void VisitPointers(HeapObject* host, Object** start, Object** end) override {
    for (Object** p = start; p < end; p++) {
      if (!(*p)->IsHeapObject())
        continue;
      auto obj = HeapObject::cast(*p);
      auto chunk = MemoryChunk::FromAddress(reinterpret_cast<Address>(obj));
      // FIXME: double check these conditions
      if (!IsInterestingType(obj->map()->instance_type())) {
        DCHECK(chunk->NeverEvacuate());
        continue;
      }
      if (root_index_map_.Lookup(obj) != RootIndexMap::kInvalidRootIndex)
        continue;
      if (IsInLoadedJso(chunk))
        continue;
      CHECK(!obj->IsExternalString());
      if (MarkObject(obj)) {
        // reset BytecodeArray age field.
        if (obj->map()->instance_type() == BYTECODE_ARRAY_TYPE) {
          BytecodeArray::cast(*p)->set_bytecode_age(BytecodeArray::kNoAgeBytecodeAge);
        }
      }
      auto current_chunk =
          MemoryChunk::FromAddress(reinterpret_cast<Address>(current_object_));
      if (current_chunk->owner()->identity() == SHARED_LO_SPACE &&
          chunk->owner()->identity() != SHARED_LO_SPACE)
        RecordSlot(current_object_, p, *p);
    }
  }

  bool IsInterestingType(InstanceType instance_type) {
    switch (instance_type) {
#define STRING_TYPE(NAME, size, name, Name) case NAME:
      STRING_TYPE_LIST(STRING_TYPE)
#undef STRING_TYPE
      case BYTE_ARRAY_TYPE:
      case BYTECODE_ARRAY_TYPE:
      case TUPLE2_TYPE:
      case FIXED_ARRAY_TYPE:
      case FIXED_DOUBLE_ARRAY_TYPE:
      case HEAP_NUMBER_TYPE:
      case SCRIPT_TYPE:
      case SHARED_FUNCTION_INFO_TYPE:
      case WEAK_CELL_TYPE:
      case SYMBOL_TYPE:
        return true;
      default:
        return false;
    }
  }

  // The correctness of this depends on JsoRange::ShouldAllocateInJsoRange().
  bool IsInLoadedJso(MemoryChunk* chunk) {
    // The const_cast here is because VirtualMemory::IsReserved() is not
    // declared as const, which should be.
    if (!const_cast<JsoRange*>(jso_range_)->Contains(chunk->address()))
      return false;
    auto result = jso_range_->FindJsoFileHeader(
        [chunk](const JsoFileHeader* header) -> bool {
          return header->IsChunkInThisFile(chunk);
        });
    return result != nullptr;
  }

  bool MarkObject(HeapObject* obj) {
    if (ObjectMarking::IsWhite(obj, MarkingState::Internal(obj))) {
      ObjectMarking::WhiteToBlack(obj, MarkingState::Internal(obj));
      marking_stack_.Add(obj);
      return true;
    }
    return false;
  }

  void RecordSlot(HeapObject* object, Object** slot, Object* target) {
    auto target_chunk = MemoryChunk::FromAddress(reinterpret_cast<Address>(target));
    auto source_chunk = MemoryChunk::FromAddress(reinterpret_cast<Address>(object));
    (void)target_chunk;  // supress "unused variable" warning
    DCHECK(target_chunk->owner()->identity() != SHARED_LO_SPACE);
    RememberedSet<OLD_TO_OLD>::Insert(source_chunk, reinterpret_cast<Address>(slot));
  }

 private:
  List<HeapObject*> marking_stack_;
  std::set<Page*> marked_pages_;
  std::set<LargePage*> marked_large_pages_;
  HeapObject* current_object_ = nullptr;
  const JsoRange* jso_range_;
  RootIndexMap root_index_map_;
};

class JsoGenerator::RecordMigratedSlotVisitor final : public ObjectVisitor {
 public:
  void VisitPointers(HeapObject* host, Object** start, Object** end) final {
    for (; start < end; start++) {
      RecordMigratedSlot(*start, reinterpret_cast<Address>(start));
    }
  }

 private:
  void RecordMigratedSlot(Object* value, Address slot) {
    if (value->IsHeapObject()) {
      auto chunk = MemoryChunk::FromAddress(reinterpret_cast<Address>(value));
      if (chunk->owner()->identity() != SHARED_LO_SPACE) {
        RememberedSet<OLD_TO_OLD>::Insert(MemoryChunk::FromAddress(slot), slot);
      }
    }
  }
};

JsoGenerator::JsoGenerator(v8::Isolate* isolate)
    : isolate_(reinterpret_cast<Isolate*>(isolate)),
      heap_(isolate_->heap()),
      aot_space_(heap_, SHARED_OLD_SPACE, Executability::NOT_EXECUTABLE) {
  aot_space_.mark_shared();
  CHECK(isolate_->jso_range_enabled());
  isolate_->set_is_doing_aot_compilation(true);
}

JsoGenerator::~JsoGenerator() {
  isolate_->set_is_doing_aot_compilation(false);
}

void JsoGenerator::Compile(ScriptCompiler::Source& source) {
  auto v8_isolate = reinterpret_cast<v8::Isolate*>(isolate_);
  auto result = ScriptCompiler::CompileUnboundScript(v8_isolate, &source);
  if (!result.IsEmpty()) {
    auto script = result.ToLocalChecked();
    unbound_scripts_.push_back(Utils::OpenHandle(*script));
  }
}

void JsoGenerator::Generate() {
  HeapIterator make_heap_iterable_and_ensure_no_allocation(heap_);
  heap_->SetGCState(Heap::AOT_MARK_MIGRATE);
  MarkingVisitor marking(isolate_);
  for (auto it : unbound_scripts_) {
    Handle<SharedFunctionInfo> sfi = Handle<SharedFunctionInfo>::cast(it);
    if (!sfi->HasBytecodeArray()) {
      PrintF("skip not has BytecodeArray SharedFuntionInfo \n");
      continue;
    }
    marking.AddRoot(*sfi);
  }
  marking.TransitiveClosure();
  marking.ForEachMarkedPage([this](Page* page) {
      ClassifyObjectsInPage(page);
    });
  MigrateAllMarkedObjects();
  for (auto page : aot_space_) {
    UpdateSlotsAndFinalize(page);
    normal_object_pages_.push_back(page);
  }
  marking.ForEachMarkedLargePage([this](LargePage* page) {
      auto object = page->GetObject();
      if (object->IsInternalizedString())
        internalized_strings_.push_back(object);
      MarkingState::Internal(page).ClearLiveness();
      UpdateSlotsAndFinalize(page);
      large_object_pages_.push_back(page);
    });
  for (auto it : unbound_scripts_)
    UpdateSlot<AccessMode::NON_ATOMIC>(it.location());
  RecoverMigratedObjects();
  heap_->shared_old_space()->MergeCompactionSpace(&aot_space_);
  heap_->SetGCState(Heap::NOT_IN_GC);
#ifdef VERIFY_HEAP
  if (FLAG_verify_heap)
    heap_->Verify();
#endif
}

namespace {

template<typename T, typename Callback = void* (void*)>
void WritePointers(std::ostream& os, T& pointers,
                   Callback ToPointer = [](void* p) -> void* { return p; }) {
  for (auto ptr : pointers) {
    auto address = (intptr_t)ToPointer(ptr);
    os.write(reinterpret_cast<char*>(&address), sizeof(address));
  }
}

template<typename T>
void WriteChunks(std::ostream& os, T& chunks) {
  for (auto chunk : chunks)
    os.write(reinterpret_cast<char*>(chunk), chunk->size());
}

}  // namespace

void JsoGenerator::Output(std::ostream& os) {
  JsoFileHeader header;
  header.num_internalized_strings = (uint32_t)internalized_strings_.size();
  header.num_unbound_scripts = (uint32_t)unbound_scripts_.size();
  header.num_normal_object_pages = (uint32_t)normal_object_pages_.size();
  header.num_large_object_pages = (uint32_t)large_object_pages_.size();
  header.heap_checksum = ComputeHeapChecksum(isolate_);
  os.write(reinterpret_cast<char*>(&header), sizeof(header));

  WritePointers(os, unbound_scripts_, [](Handle<Object> h) -> void* { return *h; });
  WritePointers(os, internalized_strings_);
  WritePointers(os, normal_object_pages_);
  WritePointers(os, large_object_pages_);

  for (auto n = header.MemoryChunkStartOffset() - header.HeaderSize(); n > 0; n--)
    os.put(0);

  WriteChunks(os, normal_object_pages_);
  WriteChunks(os, large_object_pages_);
}

void JsoGenerator::Print(std::ostream& os) {
  HeapObject* object = nullptr;
  os << "HeapCheckSum: " << (void*)ComputeHeapChecksum(isolate_) << '\n';
  for (auto script : unbound_scripts_)
    os << "UnboundScript: " << Brief(*script) << ' '
       << Brief(Script::cast(SharedFunctionInfo::cast(*script)->script())->name())
       << '\n';
  for (auto string : internalized_strings_)
    os << "Internalized string: " << Brief(string) << '\n';
  for (auto page : *heap_->shared_old_space())
    if (page->IsFlagSet(MemoryChunk::AOT_COMPILED)) {
      os << "Page@" << static_cast<void*>(page->address())
         << " ---------------------------------------------------\n";
      for (HeapObjectIterator it(page, nullptr); (object = it.Next()) != nullptr;)
        os << "  " << Brief(object) << '\n';
    }
  for (auto page : *heap_->lo_space())
    if (page->IsFlagSet(MemoryChunk::AOT_COMPILED)) {
      os << "Large object\n  " << Brief(page->GetObject()) << '\n';
    }
}

void JsoGenerator::ClassifyObjectsInPage(MemoryChunk* page) {
  for (auto object_and_size :
       LiveObjectRange<kBlackObjects>(page, MarkingState::Internal(page))) {
    HeapObject* const object = object_and_size.first;
    DCHECK(ObjectMarking::IsBlack(object, MarkingState::Internal(object)));
    if (IsMutable(object)) {
      if (object->IsWeakCell()) {
        weakcell_objects_.push_back(object);
      } else {
        mutable_objects_.push_back(object);
      }
    } else {
      immutable_objects_.push_back(object);
    }
  }
  MarkingState::Internal(page).ClearLiveness();
}

void JsoGenerator::MigrateAllMarkedObjects() {
  for (auto obj : weakcell_objects_) {
    MigrateObject(obj);
  }
  // Adjust the allocation top to aligned os page size
  Address last_address_weakcell_object = aot_space_.top();
  Address new_top = RoundUp(aot_space_.top(), base::OS::CommitPageSize());
  int remain_in_weakcell_page = static_cast<int>(new_top - last_address_weakcell_object);
  if (remain_in_weakcell_page > 0) {
    int remain_in_cur_os_page = remain_in_weakcell_page;
    while (remain_in_cur_os_page >= kMaxRegularHeapObjectSize) {
      auto res = aot_space_.AllocateRaw(kMaxRegularHeapObjectSize, kWordAligned);
      (void)(res);
      remain_in_cur_os_page -= kMaxRegularHeapObjectSize;
    }

    if (remain_in_cur_os_page > 0) {
      auto left_res = aot_space_.AllocateRaw(remain_in_cur_os_page, kWordAligned);
      (void)(left_res);
    }
  }

  for (auto obj : mutable_objects_) {
    MigrateObject(obj);
  }
  // Adjust the allocation top to aligned os page size
  Address last_address_mutable_object = aot_space_.top();
  new_top = RoundUp(aot_space_.top(), base::OS::CommitPageSize());

  int remain_in_mutable_object_page = static_cast<int>(new_top - last_address_mutable_object);
  if (remain_in_mutable_object_page > 0) {
    int remain_in_cur_os_page = remain_in_mutable_object_page;
    while (remain_in_cur_os_page >= kMaxRegularHeapObjectSize) {
      auto res = aot_space_.AllocateRaw(kMaxRegularHeapObjectSize, kWordAligned);
      (void)(res);
      remain_in_cur_os_page -= kMaxRegularHeapObjectSize;
    }

    if (remain_in_cur_os_page > 0) {
      auto left_res = aot_space_.AllocateRaw(remain_in_cur_os_page, kWordAligned);
      (void)(left_res);
    }
  }

  for (auto obj : immutable_objects_) {
    MigrateObject(obj);
  }

  if (remain_in_weakcell_page) {
    aot_space_.Free(last_address_weakcell_object, remain_in_weakcell_page);
  }
  if (remain_in_mutable_object_page) {
    aot_space_.Free(last_address_mutable_object, remain_in_mutable_object_page);
  }
}

void JsoGenerator::MigrateObject(HeapObject* src) {
  const auto size = src->Size();
  const auto alignment = src->RequiredAlignment();
  auto allocation_result = aot_space_.AllocateRaw(size, alignment);
  if (allocation_result.IsRetry()) {
    V8::FatalProcessOutOfMemory("JsoGenerator::MigrateObjectsInPage");
  }

  const auto dst = allocation_result.ToObjectChecked();

  Address dst_addr = dst->address();
  Address src_addr = src->address();
  DCHECK(heap_->AllowedToBeMigrated(src, SHARED_OLD_SPACE) ||
        MemoryChunk::FromAddress(src_addr)->owner()->identity() == MUTABLE_SPACE);
  DCHECK_OBJECT_SIZE(size);
  DCHECK(IsAligned(size, kPointerSize));
  to_be_recovered_.push_back(std::make_pair(src_addr, src->map_word()));
  heap_->CopyBlock(dst_addr, src_addr, size);
  RecordMigratedSlotVisitor visitor;
  dst->IterateBody(dst->map()->instance_type(), size, &visitor);
  Memory::Address_at(src_addr) = dst_addr;
  if (dst->IsInternalizedString())
    internalized_strings_.push_back(dst);
}

bool JsoGenerator::IsMutable(HeapObject* object) {
  switch (object->map()->instance_type()) {
    case SHARED_FUNCTION_INFO_TYPE:
    case SCRIPT_TYPE:
    case WEAK_CELL_TYPE:
      return true;
    default:
      return false;
  }
}

void JsoGenerator::UpdateSlotsAndFinalize(MemoryChunk* chunk) {
  RememberedSet<OLD_TO_OLD>::Iterate(
      chunk,
      [](Address slot) {
      return UpdateSlot<AccessMode::NON_ATOMIC>(
        reinterpret_cast<Object**>(slot));
      },
      SlotSet::PREFREE_EMPTY_BUCKETS);
  chunk->SetFlag(MemoryChunk::AOT_COMPILED);
  chunk->SetFlag(MemoryChunk::NEVER_EVACUATE);
}

template <AccessMode access_mode>
inline SlotCallbackResult JsoGenerator::UpdateSlot(Object** slot) {
  Object* obj = reinterpret_cast<Object*>(
      base::Relaxed_Load(reinterpret_cast<base::AtomicWord*>(slot)));

  if (obj->IsHeapObject()) {
    HeapObject* heap_obj = HeapObject::cast(obj);
    MapWord map_word = heap_obj->map_word();
    if (map_word.IsForwardingAddress()) {
      HeapObject* target = map_word.ToForwardingAddress();
      if (access_mode == AccessMode::NON_ATOMIC) {
        *slot = target;
      } else {
        base::AsAtomicWord::Release_CompareAndSwap(slot, obj, target);
      }
    }
  }
  return REMOVE_SLOT;
}

void JsoGenerator::RecoverMigratedObjects() {
  for (auto it : to_be_recovered_) {
    Memory::Address_at(it.first) = reinterpret_cast<Address>(it.second.ToMap());
  }
}

}  // namespace internal
}  // namespace v8
