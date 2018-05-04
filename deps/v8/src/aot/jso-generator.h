// Copyright 2017 the Alibaba YunOS project.  All rights reserved.

#ifndef V8_AOT_JSO_GENERATOR_H_
#define V8_AOT_JSO_GENERATOR_H_

#include <ostream>
#include <vector>
#include "src/aot/jso-common.h"
#include "src/heap/mark-compact.h"
#include "src/heap/slot-set.h"

namespace v8 {
namespace internal {

class AotCompactionSpace  : public CompactionSpace {
 public:
  AotCompactionSpace(Heap* heap, AllocationSpace id, Executability executable)
      : CompactionSpace(heap, id, executable) {}

  bool is_local() override { return false; }
};

class JsoGenerator : public JsoCommonStatic {
 public:
  explicit JsoGenerator(v8::Isolate* isolate);
  ~JsoGenerator();

  void Compile(ScriptCompiler::Source& source);

  void Generate();

  void Output(std::ostream& os);

  void Print(std::ostream& os);

 private:
  class MarkingVisitor;
  class RecordMigratedSlotVisitor;

 private:
  void ClassifyObjectsInPage(MemoryChunk* page);
  void MigrateAllMarkedObjects(void);
  void MigrateObject(HeapObject* src);

  static bool IsMutable(HeapObject* object);

  static void UpdateSlotsAndFinalize(MemoryChunk* chunk);

  template <AccessMode access_mode>
  static SlotCallbackResult UpdateSlot(Object** slot);

  void RecoverMigratedObjects();

 private:
  Isolate* isolate_;
  Heap* heap_;
  AotCompactionSpace aot_space_;
  std::vector<Handle<Object>> unbound_scripts_;
  std::vector<Object*> internalized_strings_;
  std::vector<HeapObject*> weakcell_objects_;
  std::vector<HeapObject*> mutable_objects_;
  std::vector<HeapObject*> immutable_objects_;
  std::vector<Page*> normal_object_pages_;
  std::vector<LargePage*> large_object_pages_;
  std::vector<std::pair<Address, MapWord>> to_be_recovered_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_AOT_JSO_GENERATOR_H_
