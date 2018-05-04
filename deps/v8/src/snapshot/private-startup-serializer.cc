#include "src/snapshot/private-startup-serializer.h"
#include "src/snapshot/startup-serializer.h"

#include "src/objects-inl.h"

namespace v8 {
namespace internal {

PrivateStartupSerializer::PrivateStartupSerializer(
    Isolate* isolate, StartupSerializer* startup_serializer)
    : Serializer(isolate),
      startup_serializer_(startup_serializer),
      clear_function_code_(startup_serializer->clear_function_code_),
      can_be_rehashed_(true) {
  InitializeCodeAddressMap();
}

PrivateStartupSerializer::~PrivateStartupSerializer() {
  RestoreExternalReferenceRedirectors(&accessor_infos_);
  OutputStatistics("PrivateStartupSerializer");
}

void PrivateStartupSerializer::SerializeNoScriptSFI() {
  PutSmi(startup_serializer_->next_template_serial_number_);
  Object* infos = startup_serializer_->noscript_shared_function_infos_;
  if (infos->IsSmi()) {
    PutSmi(Smi::cast(infos));
  } else {
    DCHECK(!HeapObject::cast(infos)->is_shared_object());
    SerializeObject(HeapObject::cast(infos), kPlain, kStartOfObject, 0);
  }

  FixedArray* templates = startup_serializer_->serialized_templates_;
  if (templates != isolate()->heap()->empty_fixed_array()) {
    DCHECK(!templates->is_shared_object());
  }
  SerializeObject(HeapObject::cast(templates), kPlain, kStartOfObject, 0);
}


void PrivateStartupSerializer::Serialize() {
  Object* undefined = isolate()->heap()->undefined_value();
  VisitRootPointer(Root::kPartialSnapshotCache, &undefined);
}

void PrivateStartupSerializer::SerializeObject(HeapObject* obj, HowToCode how_to_code,
                                        WhereToPoint where_to_point, int skip) {
  DCHECK(!obj->IsJSFunction());

  if (clear_function_code_) {
    if (obj->IsCode()) {
      Code* code = Code::cast(obj);
      // If the function code is compiled (either as native code or bytecode),
      // replace it with lazy-compile builtin. Only exception is when we are
      // serializing the canonical interpreter-entry-trampoline builtin.
      if (code->kind() == Code::FUNCTION ||
           code->is_interpreter_trampoline_builtin()) {
        obj = isolate()->builtins()->builtin(Builtins::kCompileLazy);
      }
    } else if (obj->IsBytecodeArray()) {
      obj = isolate()->heap()->undefined_value();
    }
  } else if (obj->IsCode()) {
    Code* code = Code::cast(obj);
    if (code->kind() == Code::FUNCTION) {
      code->ClearInlineCaches();
    }
  }

  if (SerializeHotObject(obj, how_to_code, where_to_point, skip)) return;

  int root_index = root_index_map_.Lookup(obj);
  if (root_index != RootIndexMap::kInvalidRootIndex) {
    PutRoot(root_index, obj, how_to_code, where_to_point, skip);
    return;
  }

  if (SerializeBackReference(obj, how_to_code, where_to_point, skip)) return;

  if (startup_serializer_->ShouldBeInThePartialSnapshotCache(obj)) {
    FlushSkip(skip);
    int cache_index = startup_serializer_->PartialSnapshotCacheIndex(obj);
    sink_.Put(kPartialSnapshotCache + how_to_code + where_to_point,
              "PartialSnapshotCache");
    sink_.Put(static_cast<byte>(PartialSnapshotCacheType::kShared), "partial_snapshot_cache_type");
    sink_.PutInt(cache_index, "partial_snapshot_cache_index");
    return;
  }

  DCHECK(!startup_serializer_->reference_map()->Lookup(obj).is_valid());

  FlushSkip(skip);

  if (isolate_->external_reference_redirector() && obj->IsAccessorInfo()) {
    // Wipe external reference redirects in the accessor info.
    AccessorInfo* info = AccessorInfo::cast(obj);
    Address original_address = Foreign::cast(info->getter())->foreign_address();
    Foreign::cast(info->js_getter())->set_foreign_address(original_address);
    accessor_infos_.Add(info);
  } else if (obj->IsScript() && Script::cast(obj)->IsUserJavaScript()) {
    Script::cast(obj)->set_context_data(
        isolate_->heap()->uninitialized_symbol());
  }

  if (obj->IsHashTable()) CheckRehashability(obj);

  // Object has not yet been serialized.  Serialize it here.
  ObjectSerializer object_serializer(this, obj, &sink_, how_to_code,
                                     where_to_point);
  object_serializer.Serialize();
}

int PrivateStartupSerializer::PartialSnapshotCacheIndex(HeapObject* heap_object) {
  int index;
  if (!partial_cache_index_map_.LookupOrInsert(heap_object, &index)) {
    SerializeObject(heap_object, kPlain, kStartOfObject, 0);
  }
  return index;
}

bool PrivateStartupSerializer::ShouldBeInThePartialSnapshotCache(HeapObject* o) {
  // Scripts should be referred only through shared function infos.  We can't
  // allow them to be part of the partial snapshot because they contain a
  // unique ID, and deserializing several partial snapshots containing script
  // would cause dupes.
  DCHECK(!o->IsScript());
  if (startup_serializer_->ShouldBeInThePartialSnapshotCache(o)) return false;
  return o->IsSharedFunctionInfo() || o->IsHeapNumber() ||
         o->IsScopeInfo() || o->IsAccessorInfo() ||
         o->IsTemplateInfo() ||
         o->map() ==
             startup_serializer_->isolate()->heap()->fixed_cow_array_map();
}

void PrivateStartupSerializer::CheckRehashability(HeapObject* table) {
  DCHECK(table->IsHashTable());
  if (!can_be_rehashed_) return;
  // We can only correctly rehash if the four hash tables below are the only
  // ones that we deserialize.
  DCHECK(isolate_->heap()->weak_wasm_data_table() == Smi::kZero);
  DCHECK(isolate_->heap()->weak_debug_points_table() == Smi::kZero);
  if (table == isolate_->heap()->empty_slow_element_dictionary()) return;
  if (table == isolate_->heap()->empty_property_dictionary()) return;
  if (table == isolate_->heap()->weak_object_to_code_table()) return;
  if (table == isolate_->heap()->string_table()) return;
  can_be_rehashed_ = false;
}

}  // namespace internal
}  // namespace v8
