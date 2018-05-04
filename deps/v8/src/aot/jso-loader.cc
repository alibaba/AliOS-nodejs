// Copyright 2017 the Alibaba YunOS project.  All rights reserved.

#include "src/aot/jso-loader.h"
#include <vector>
#include "src/assembler-inl.h"
#include "src/objects-inl.h"
#include "src/thread/worker-runtime.h"

#include <sys/mman.h>
#include <unistd.h>
#include <src/utils.h>

namespace v8 {
namespace internal {

JsoLoader::JsoLoader(v8::Isolate* isolate)
    : isolate_(reinterpret_cast<Isolate*>(isolate)) {
}

int JsoLoader::Load(int fd, bool invertedReplaceFlag) {
  if (!isolate_->jso_range_enabled()) {
    Throw("JsoRange is not enabled");
    return -1;
  }
  JsoFileHeader header;
  if (read(fd, &header, sizeof(header)) < 0) {
    Throw("failed to read jso file");
    return -1;
  }
  header_ = (JsoFileHeader*)mmap(nullptr, header.HeaderSize(),
                                 PROT_READ, MAP_PRIVATE, fd, 0);
  if (header_ == MAP_FAILED) {
    Throw("invalid jso file");
    return -1;
  }
  if (header_->heap_checksum != ComputeHeapChecksum(isolate_)) {
    Throw("incorrect heap checksum");
    munmap((void*)header_, header.HeaderSize());
    header_ = nullptr;
    return -1;
  }
  if (HasBeenLoaded(header_)) {
    Throw("jso file has been loaded or conflict addresses");
    munmap((void*)header_, header.HeaderSize());
    header_ = nullptr;
    return -1;
  }
  // Once loaded pages are linked to heap, GC is not allowed until handles are
  // created for scripts.
  {
    // HeapIterator make_heap_iterable_and_ensure_no_allocation(isolate_->heap());
#if defined(V8_CAN_USE_LOGCAT)
    v8::internal::PrintF("Starting loading jso\n");
#endif
    {
      DisallowHeapAllocation no_heap_allocation;
      MapAndLinkPages(fd);
    }
    unbound_scripts_.reserve(header_->num_unbound_scripts);
    for (size_t i = 0; i < header_->num_unbound_scripts; i++) {
      auto info = SharedFunctionInfo::cast(header_->unbound_scripts()[i]);
      unbound_scripts_.push_back(Handle<SharedFunctionInfo>(info));
      if (v8::internal::FLAG_test_d8 &&
          info->script()->IsScript()) {
        Heap* heap = isolate_->heap();
        Factory* factory = isolate_->factory();
        HandleScope scope(isolate_);
        Handle<Script> script = handle(Script::cast(info->script()), isolate_);
        script->set_id(heap->NextScriptId());
        script->set_context_data(isolate_->native_context()->debug_context_id());
        {
          DCHECK(script->is_shared_object());
          WorkerRuntime* worker_runtime = isolate_->worker_runtime();
          sync::MutexLock mu(isolate_->vmthread(),
              WorkerRuntime::GetLock(worker_runtime, WorkerRuntime::kScriptList));
          Handle<Object> list = WeakFixedArray::Add(factory->shared_script_list(), script);
          heap->SetRootScriptList(*list);
        }
      }
    }
#if defined(V8_CAN_USE_LOGCAT)
    v8::internal::PrintF("Ending loading jso\n");
#endif
  }
  LinkInternalizedStrings(invertedReplaceFlag);
#ifdef VERIFY_HEAP
  if (FLAG_verify_heap)
    isolate_->heap()->Verify();
#endif
  isolate_->heap()->memory_allocator()->jso_range()->AddLoadedJsoFileHeader(header_);
  return header.num_unbound_scripts;
}

MaybeLocal<UnboundScript> JsoLoader::GetUnboundScript(unsigned i) const {
  if (i >= unbound_scripts_.size())
    return MaybeLocal<UnboundScript>();
  return ToApiHandle<UnboundScript>(unbound_scripts_[i]);
}

bool JsoLoader::HasBeenLoaded(const JsoFileHeader* header) {
  if (header->num_unbound_scripts == 0)
    return false;
  const auto addr = header->unbound_scripts()[0]->address();
  for (auto page : *isolate_->heap()->shared_old_space())
    if (page->Contains(addr))
      return true;
  return false;
}

class JsoLoaderStringTableInsertionKey : public StringTableKey {
 public:
  explicit JsoLoaderStringTableInsertionKey(String* string)
      : StringTableKey(string->hash_field()), string_(string) {
    DCHECK(string_->IsInternalizedString());
  }

  bool IsMatch(Object* string) override {
    // if (Hash() != String::cast(string)->Hash()) return false;
    return string_->SlowEquals(String::cast(string));
  }

  MUST_USE_RESULT Handle<String> AsHandle(Isolate* isolate) override {
    return handle(string_, isolate);
  }

 private:
  String* string_;
  DisallowHeapAllocation no_gc;
};

namespace {

template<typename PageT>
size_t MapPages(int fd, size_t offset, PageT* const* pages, size_t num) {
  // TODO(aot): check with MemoryAllocator::jso_range_ that the address ranges
  // of pages in jso file are free and then update jso_range_ after mapping.
  for (size_t i = 0; i < num; i++) {
    auto page = (PageT*)mmap(pages[i], Page::kPageSize, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_FIXED, fd, offset);
    DCHECK(page == pages[i]);
    if (page->size() > Page::kPageSize) {
      page = (PageT*)mmap(page, page->size(), PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_FIXED, fd, offset);
      DCHECK(page == pages[i]);
    }
    offset += page->size();
  }
  return offset;
}

template<typename PageT, typename SpaceT>
void LinkPages(SpaceT* space, PageT* const* pages, size_t num) {
  for (size_t i = 0; i < num; i++) {
    space->LinkCompiledPage(pages[i]);
  }
}

}  // namespace

void JsoLoader::MapAndLinkPages(int fd) {
  auto h = header_;
  auto offset = h->MemoryChunkStartOffset();
  offset = MapPages(fd, offset, h->normal_object_pages(), h->num_normal_object_pages);
  offset = MapPages(fd, offset, h->large_object_pages(), h->num_large_object_pages);
  auto heap = isolate_->heap();
  LinkPages(heap->shared_old_space(), h->normal_object_pages(), h->num_normal_object_pages);
  LinkPages(heap->shared_lo_space(), h->large_object_pages(), h->num_large_object_pages);
}

namespace {

class InternalizedStringForwardingVisitor : public ObjectVisitor, public RootVisitor {
 private:
  void VisitPointer(HeapObject* host, Object** p) override { UpdateSlot(p); }

  void VisitPointers(HeapObject* host, Object** start, Object** end) override {
    for (Object** p = start; p < end; p++) UpdateSlot(p);
  }

  void VisitRootPointers(Root root, Object** start, Object** end) override {
    for (Object** p = start; p < end; p++) UpdateSlot(p);
  }

  void VisitEmbeddedPointer(Code* host, RelocInfo* rinfo) override {
    UpdateTypedSlotHelper::UpdateEmbeddedPointer(rinfo, UpdateSlot);
  }

  static SlotCallbackResult UpdateSlot(Object** slot) {
    if ((*slot)->IsHeapObject()) {
      auto obj = HeapObject::cast(*slot);
      if (obj->IsInternalizedString())
        *slot = String::cast(obj)->GetForwardedInternalizedString();
    }
    return REMOVE_SLOT;
  }
};

void ForwardInternalizedStrings(HeapObject* obj) {
  InternalizedStringForwardingVisitor visitor;
  obj->IterateBody(obj->map()->instance_type(), obj->Size(), &visitor);
}

void ForwardInternalizedStrings(Page* page) {
  HeapObjectIterator it(page, nullptr);
  for (HeapObject* obj; (obj = it.Next()) != nullptr;)
    ForwardInternalizedStrings(obj);
}

template<typename SpaceType>
void ForwardInternalizedStrings(SpaceType* space) {
  for (auto page : *space) {
    if (page->IsFlagSet(MemoryChunk::AOT_COMPILED))
      continue;
    HeapObjectIterator it(page, nullptr);
    for (HeapObject* obj; (obj = it.Next()) != nullptr;)
      ForwardInternalizedStrings(obj);
  }
}

}  // namespace

void JsoLoader::LinkInternalizedStrings(bool flipped) {
  bool has_existing_string = false;
  auto internalized_strings = header_->internalized_strings();
  std::vector<bool> is_new(header_->num_internalized_strings, false);
  int new_string_count = 0;

  for (size_t i = 0; i < header_->num_internalized_strings; i++) {
    HandleScope scope(isolate_);
    auto string = internalized_strings[i];
    JsoLoaderStringTableInsertionKey key(string);
    auto result = StringTable::LookupKeyIfExists(isolate_, &key);
    if (result) {
      has_existing_string = true;
      if (flipped) {
        string->SetForwardedInternalizedString(result);
      } else {
        result->SetForwardedInternalizedString(string);
      }
    } else {
      is_new[i] = true;
      new_string_count++;
    }
  }

  if (has_existing_string) {
    auto heap = isolate_->heap();
    if (!flipped) {
      InternalizedStringForwardingVisitor visitor;
      heap->IterateRoots(&visitor, VISIT_ALL);
      SemiSpaceIterator it(heap->new_space());
      for (HeapObject* obj; (obj = it.Next()) != nullptr;)
        ForwardInternalizedStrings(obj);
      ForwardInternalizedStrings(heap->mutable_space());
      ForwardInternalizedStrings(heap->old_space());
      ForwardInternalizedStrings(heap->code_space());
      ForwardInternalizedStrings(heap->map_space());
      ForwardInternalizedStrings(heap->shared_old_space());
      ForwardInternalizedStrings(heap->shared_code_space());
      ForwardInternalizedStrings(heap->shared_map_space());
      for (auto page : *heap->lo_space()) {
        if (page->IsFlagSet(MemoryChunk::AOT_COMPILED))
          continue;
        ForwardInternalizedStrings(page->GetObject());
      }
      for (auto page : *heap->shared_lo_space()) {
        if (page->IsFlagSet(MemoryChunk::AOT_COMPILED))
          continue;
        ForwardInternalizedStrings(page->GetObject());
      }
    } else {
      auto h = header_;
      auto normal_pages = h->normal_object_pages();
      for (size_t i = 0; i < h->num_normal_object_pages; ++i) {
        ForwardInternalizedStrings(normal_pages[i]);
      }
      auto large_object_pages = h->large_object_pages();
      for (size_t i = 0; i < h->num_large_object_pages; ++i) {
        ForwardInternalizedStrings(large_object_pages[i]->GetObject());
      }
    }
  }

  StringTable::EnsureCapacityForDeserialization(isolate_, new_string_count);
  for (size_t i = 0; i < header_->num_internalized_strings; i++) {
    if (!is_new[i])
      continue;
    HandleScope scope(isolate_);
    JsoLoaderStringTableInsertionKey key(internalized_strings[i]);
    StringTable::LookupKey(isolate_, &key);
  }
}

Local<Value> JsoLoader::Throw(const char* message) {
  auto isolate = reinterpret_cast<v8::Isolate*>(isolate_);
  return isolate->ThrowException(
      v8::String::NewFromUtf8(isolate, message, NewStringType::kNormal)
      .ToLocalChecked());
}

}  // namespace internal
}  // namespace v8
