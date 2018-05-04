#ifndef THREAD_SERIALIZATION_DATA_H_
#define THREAD_SERIALIZATION_DATA_H_

#include "include/v8.h"
#include "src/utils.h"
#include "src/objects.h"

namespace v8 {
namespace worker {

class ExternalizedContents;

class SerializationData {
 public:
  SerializationData() : size_(0) {}

  uint8_t* data() { return data_.get(); }
  size_t size() { return size_; }
  const std::vector<ArrayBuffer::Contents>& array_buffer_contents() {
    return array_buffer_contents_;
  }
  const std::vector<SharedArrayBuffer::Contents>&
  shared_array_buffer_contents() {
    return shared_array_buffer_contents_;
  }

  void AppendExternalizedContentsTo(std::vector<ExternalizedContents>* to) {
    to->insert(to->end(),
               std::make_move_iterator(externalized_contents_.begin()),
               std::make_move_iterator(externalized_contents_.end()));
    externalized_contents_.clear();
  }

 private:
  struct DataDeleter {
    void operator()(uint8_t* p) const { free(p); }
  };

  std::unique_ptr<uint8_t, DataDeleter> data_;
  size_t size_;
  std::vector<ArrayBuffer::Contents> array_buffer_contents_;
  std::vector<SharedArrayBuffer::Contents> shared_array_buffer_contents_;
  std::vector<ExternalizedContents> externalized_contents_;

 private:
  friend class Serializer;

  DISALLOW_COPY_AND_ASSIGN(SerializationData);
};


// The backing store of an ArrayBuffer or SharedArrayBuffer, after
// Externalize() has been called on it.
class ExternalizedContents {
 public:
  explicit ExternalizedContents(const ArrayBuffer::Contents& contents)
      : data_(contents.Data()), size_(contents.ByteLength()) {}
  explicit ExternalizedContents(const SharedArrayBuffer::Contents& contents)
      : data_(contents.Data()), size_(contents.ByteLength()) {}
  ExternalizedContents(ExternalizedContents&& other)
      : data_(other.data_), size_(other.size_) {
    other.data_ = nullptr;
    other.size_ = 0;
  }
  ExternalizedContents& operator=(ExternalizedContents&& other) {
    if (this != &other) {
      data_ = other.data_;
      size_ = other.size_;
      other.data_ = nullptr;
      other.size_ = 0;
    }
    return *this;
  }

  void FreeData(ArrayBuffer::Allocator* array_buffer_allocator) const;

 private:
  void* data_;
  size_t size_;

  DISALLOW_COPY_AND_ASSIGN(ExternalizedContents);
};


class WorkerSerialization {
 public:
  explicit WorkerSerialization(ArrayBuffer::Allocator* array_buffer_allocator);

  SerializationData* SerializeValue(Isolate* isolate, Local<Value> value);
  SerializationData* SerializeValue(Isolate* isolate, Local<Value> value,
                                   Local<Array> transfer);
  MaybeLocal<Value> DeserializeValue(Isolate* isolate, SerializationData* data);

  void CleanupExternalizedSharedContents();
  void CleanupSerializationData(SerializationData* data);
 private:
  WorkerSerialization(const WorkerSerialization&) = delete;
  void operator=(const WorkerSerialization&) = delete;


  ArrayBuffer::Allocator* array_buffer_allocator_;
  std::vector<ExternalizedContents> externalized_contents_;
  base::Mutex mutex_;
};

}  // namespace worker
}  // namespace v8

#endif  // THREAD_SERIALIZATION_DATA_H_
