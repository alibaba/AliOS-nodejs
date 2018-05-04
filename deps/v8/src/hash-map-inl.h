#ifndef V8_HASH_MAP_INL_H
#define V8_HASH_MAP_INL_H
#include "src/hash-map.h"
#include "src/objects-inl.h"
#include "src/isolate.h"

namespace v8 {
namespace internal {
template <typename Shape, typename Key>
GcFreeHashMap<Shape, Key>::GcFreeHashMap(Isolate* isolate, uint32_t size)
                  : main_isolate_(isolate),
                    key_array_(NULL),
                    size_(Smi::FromInt(size)),
                    number_of_elements_(Smi::FromInt(0)) {
   if (size > 0) {
     key_array_ = reinterpret_cast<Object**>(Malloced::New(size* sizeof(Object*)));
     memset(key_array_, kSmiTag, size * sizeof(Object*));
   }
}

template <typename Shape, typename Key>
GcFreeHashMap<Shape, Key>::~GcFreeHashMap() {
  if (key_array_ != NULL) {
    Malloced::Delete(key_array_);
  }
  key_array_ = NULL;
  number_of_elements_ = Smi::FromInt(0);
  size_ = Smi::FromInt(0);
}

template <typename Shape, typename Key>
void GcFreeHashMap<Shape, Key>::ElementsRemoved(int n) {
  CHECK(number_of_elements_->value() >= n);
  number_of_elements_ = Smi::FromInt(number_of_elements_->value() - n);
}


template <typename Shape, typename Key>
uint32_t GcFreeHashMap<Shape, Key>::Hash(Key key) {
  return Shape::Hash(key);
}

template <typename Shape, typename Key>
uint32_t GcFreeHashMap<Shape, Key>::HashForObject(Key key, Object* object) {
  return Shape::HashForObject(key, object);
}

template <typename Shape, typename Key>
int GcFreeHashMap<Shape, Key>::FindEntry(Key key) {
  return FindEntry(key, Hash(key));
}

template <typename Shape, typename Key>
uint32_t GcFreeHashMap<Shape, Key>::FindInsertEntry(uint32_t hash) {
  uint32_t entry = FirstProbe(hash, Capacity());
  int count = 1;
  while (true) {
    Object* element = KeyAt(entry);
    if (!IsKey(element)) break;
    entry = NextProbe(entry, count++, Capacity());
  }
  return entry;
}

template <typename Shape, typename Key>
void GcFreeHashMap<Shape, Key>::SetEntry(uint32_t entry, Object* o) {
  key_array_[entry] = o;
}

template <typename Shape, typename Key>
void GcFreeHashMap<Shape, Key>::ElementAdded() {
  number_of_elements_ = Smi::FromInt(number_of_elements_->value() + 1);
}

template <typename Shape, typename Key>
Object* GcFreeHashMap<Shape, Key>::KeyAt(int index) {
  DCHECK(index < static_cast<int>(Capacity()));
  return key_array_[index];
}

template <typename Shape, typename Key>
uint32_t GcFreeHashMap<Shape, Key>::Capacity() {
  return size_->value();
}

template <typename Shape, typename Key>
bool GcFreeHashMap<Shape, Key>::IsKey(Object* k) {
  if(k->IsSmi()) {
    return Smi::cast(k)->value() != 0;
  }

  return k != main_isolate_->heap()->the_hole_value();
}

template <typename Shape, typename Key>
bool GcFreeHashMap<Shape, Key>::IsFree(Object* k) {
  if (k->IsSmi()) {
    return Smi::cast(k)->value() == 0;
  }
  return false;
}

template <typename Shape, typename Key>
int GcFreeHashMap<Shape, Key>::FindEntry(Key key, uint32_t hash) {
  uint32_t entry_index = FirstProbe(hash, Capacity());
  uint32_t count = 1;
  Object* the_hole = main_isolate_->heap()->the_hole_value();
  while (true) {
    Object* element = key_array_[entry_index];
    if (IsFree(element)) break;
    if (the_hole != element && Shape::IsMatch(key, element))  return entry_index;
    // next empty entry
    entry_index = NextProbe(entry_index, count++, Capacity());
    CHECK(count < Capacity());
  }
  return kNotFound;
}

template <typename Shape, typename Key>
int GcFreeHashMap<Shape, Key>::NumberOfElements() {
  return number_of_elements_->value();
}

}  // namespace internal
}  // namespace v8
#endif
