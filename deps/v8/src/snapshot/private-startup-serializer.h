#ifndef V8_SNAPSHOT_PRIVATE_STARTUP_SERIALIZER_H_
#define V8_SNAPSHOT_PRIVATE_STARTUP_SERIALIZER_H_

#include "include/v8.h"
#include "src/snapshot/serializer.h"
#include "src/snapshot/startup-serializer.h"

namespace v8 {
namespace internal {

class StartupSerializer;

class PrivateStartupSerializer : public Serializer {
 public:
   PrivateStartupSerializer(Isolate* isolate,
       StartupSerializer* startup_serializer);

  ~PrivateStartupSerializer() override;

  void Serialize();
  void SerializeNoScriptSFI();
  void SerializeObject(HeapObject* o, HowToCode how_to_code,
                       WhereToPoint where_to_point, int skip) override;

  bool ShouldBeInThePartialSnapshotCache(HeapObject* o);
  int PartialSnapshotCacheIndex(HeapObject* o);

  bool can_be_rehashed() const { return can_be_rehashed_; }
  bool clear_function_code() const { return clear_function_code_; }

 private:
  void CheckRehashability(HeapObject* hashtable);

  StartupSerializer* startup_serializer_;
  StartupSerializer::PartialCacheIndexMap partial_cache_index_map_;
  List<AccessorInfo*> accessor_infos_;
  bool clear_function_code_;
  bool can_be_rehashed_;
  DISALLOW_COPY_AND_ASSIGN(PrivateStartupSerializer);
};

}  // namespace internal
}  // namespace v8

#endif  // V8_SNAPSHOT_PRIVATE_STARTUP_SERIALIZER_H_
