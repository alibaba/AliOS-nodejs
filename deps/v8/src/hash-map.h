#ifndef V8_HASH_MAP_H_
#define V8_HASH_MAP_H_
#include "src/allocation.h"

namespace v8 {
namespace internal {

class Isolate;
class Object;
class ObjectVisitor;
//GcFreeHashMap is a C++ struct
//so it is manmaged by malloc/free without gc
template <typename Shape, typename Key>
class GcFreeHashMap {
 public:
  static const int kNotFound = -1;

  explicit GcFreeHashMap(Isolate* isolate, uint32_t size);
  void ElementsRemoved(int n);

  virtual ~GcFreeHashMap();
  uint32_t Hash(Key key);
  uint32_t HashForObject(Key key, Object* object);
  int FindEntry(Key key);
  uint32_t FindInsertEntry(uint32_t hash);
  void SetEntry(uint32_t entry, Object* o);
  void ElementAdded();

  Object* KeyAt(int index);
  uint32_t Capacity();

  virtual bool IsKey(Object* k);
  virtual bool IsFree(Object* k);

  virtual void EnsureCapacity(Key key, uint32_t new_size) = 0;
  virtual void Iterate(RootVisitor* v, VisitMode mode) = 0;
  int NumberOfElements();

 protected:
  inline static uint32_t FirstProbe(uint32_t hash, uint32_t size) {
    return hash & (size - 1);
  }

  inline static int32_t NextProbe(
      uint32_t last, uint32_t number, uint32_t size) {
    return (last + number) & (size - 1);
  }

  int FindEntry(Key key, uint32_t hash);

  Isolate* main_isolate_;
  Object** key_array_;
  Smi* size_;
  Smi* number_of_elements_;
};

}  // namespace internal
}  // namespace v8

#endif  // V8_HASH_MAP_H_
