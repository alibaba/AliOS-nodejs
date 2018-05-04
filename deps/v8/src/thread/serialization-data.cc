#include "src/thread/serialization-data.h"
#include "src/list-inl.h"

namespace v8 {
namespace worker {

const int MB = 1024 * 1024;
const int kMaxSerializerMemoryUsage = 1 * MB;  // Arbitrary maximum for testing.

static Local<Value> Throw(Isolate* isolate, const char* message) {
  return isolate->ThrowException(
      String::NewFromUtf8(isolate, message, NewStringType::kNormal)
          .ToLocalChecked());
}

void ExternalizedContents::FreeData(ArrayBuffer::Allocator* array_buffer_allocator) const {
  array_buffer_allocator->Free(data_, size_);
}

class Serializer : public ValueSerializer::Delegate {
 public:
  explicit Serializer(Isolate* isolate)
      : isolate_(isolate),
        serializer_(isolate, this),
        current_memory_usage_(0) {}

  Maybe<bool> WriteValue(Local<Context> context, Local<Value> value,
                         Local<Value> transfer) {
    bool ok;
    DCHECK(!data_);
    data_.reset(new SerializationData);
    if (!PrepareTransfer(context, transfer).To(&ok)) {
      return Nothing<bool>();
    }
    serializer_.WriteHeader();

    if (!serializer_.WriteValue(context, value).To(&ok)) {
      data_.reset();
      return Nothing<bool>();
    }

    if (!FinalizeTransfer().To(&ok)) {
      return Nothing<bool>();
    }

    std::pair<uint8_t*, size_t> pair = serializer_.Release();
    data_->data_.reset(pair.first);
    data_->size_ = pair.second;
    return Just(true);
  }

  std::unique_ptr<SerializationData> Release() { return std::move(data_); }

 protected:
  // Implements ValueSerializer::Delegate.
  void ThrowDataCloneError(Local<String> message) override {
    isolate_->ThrowException(Exception::Error(message));
  }

  Maybe<uint32_t> GetSharedArrayBufferId(
      Isolate* isolate, Local<SharedArrayBuffer> shared_array_buffer) override {
    DCHECK(data_ != nullptr);
    for (size_t index = 0; index < shared_array_buffers_.size(); ++index) {
      if (shared_array_buffers_[index] == shared_array_buffer) {
        return Just<uint32_t>(static_cast<uint32_t>(index));
      }
    }

    size_t index = shared_array_buffers_.size();
    shared_array_buffers_.emplace_back(isolate_, shared_array_buffer);
    return Just<uint32_t>(static_cast<uint32_t>(index));
  }

  void* ReallocateBufferMemory(void* old_buffer, size_t size,
                               size_t* actual_size) override {
    // Not accurate, because we don't take into account reallocated buffers,
    // but this is fine for testing.
    current_memory_usage_ += size;
    if (current_memory_usage_ > kMaxSerializerMemoryUsage) return nullptr;

    void* result = realloc(old_buffer, size);
    *actual_size = result ? size : 0;
    return result;
  }

  void FreeBufferMemory(void* buffer) override { free(buffer); }

 private:
  Maybe<bool> PrepareTransfer(Local<Context> context, Local<Value> transfer) {
    if (transfer->IsArray()) {
      Local<Array> transfer_array = Local<Array>::Cast(transfer);
      uint32_t length = transfer_array->Length();
      for (uint32_t i = 0; i < length; ++i) {
        Local<Value> element;
        if (transfer_array->Get(context, i).ToLocal(&element)) {
          if (!element->IsArrayBuffer()) {
            Throw(isolate_, "Transfer array elements must be an ArrayBuffer");
            break;
          }

          Local<ArrayBuffer> array_buffer = Local<ArrayBuffer>::Cast(element);
          serializer_.TransferArrayBuffer(
              static_cast<uint32_t>(array_buffers_.size()), array_buffer);
          array_buffers_.emplace_back(isolate_, array_buffer);
        } else {
          return Nothing<bool>();
        }
      }
      return Just(true);
    } else if (transfer->IsUndefined()) {
      return Just(true);
    } else {
      Throw(isolate_, "Transfer list must be an Array or undefined");
      return Nothing<bool>();
    }
  }

  template <typename T>
  typename T::Contents MaybeExternalize(Local<T> array_buffer) {
    if (array_buffer->IsExternal()) {
      return array_buffer->GetContents();
    } else {
      typename T::Contents contents = array_buffer->Externalize();
      data_->externalized_contents_.emplace_back(contents);
      return contents;
    }
  }

  Maybe<bool> FinalizeTransfer() {
    for (const auto& global_array_buffer : array_buffers_) {
      Local<ArrayBuffer> array_buffer =
          Local<ArrayBuffer>::New(isolate_, global_array_buffer);
      if (!array_buffer->IsNeuterable()) {
        Throw(isolate_, "ArrayBuffer could not be transferred");
        return Nothing<bool>();
      }

      ArrayBuffer::Contents contents = MaybeExternalize(array_buffer);
      array_buffer->Neuter();
      data_->array_buffer_contents_.push_back(contents);
    }

    for (const auto& global_shared_array_buffer : shared_array_buffers_) {
      Local<SharedArrayBuffer> shared_array_buffer =
          Local<SharedArrayBuffer>::New(isolate_, global_shared_array_buffer);
      data_->shared_array_buffer_contents_.push_back(
          MaybeExternalize(shared_array_buffer));
    }

    return Just(true);
  }

  Isolate* isolate_;
  ValueSerializer serializer_;
  std::unique_ptr<SerializationData> data_;
  std::vector<Global<ArrayBuffer>> array_buffers_;
  std::vector<Global<SharedArrayBuffer>> shared_array_buffers_;
  size_t current_memory_usage_;

  DISALLOW_COPY_AND_ASSIGN(Serializer);
};

class Deserializer : public ValueDeserializer::Delegate {
 public:
  Deserializer(Isolate* isolate, std::unique_ptr<SerializationData> data)
      : isolate_(isolate),
        deserializer_(isolate, data->data(), data->size(), this),
        data_(std::move(data)) {
    deserializer_.SetSupportsLegacyWireFormat(true);
  }

  MaybeLocal<Value> ReadValue(Local<Context> context) {
    bool read_header;
    if (!deserializer_.ReadHeader(context).To(&read_header)) {
      return MaybeLocal<Value>();
    }

    uint32_t index = 0;
    for (const auto& contents : data_->array_buffer_contents()) {
      Local<ArrayBuffer> array_buffer =
          ArrayBuffer::New(isolate_, contents.Data(), contents.ByteLength());
      deserializer_.TransferArrayBuffer(index++, array_buffer);
    }

    index = 0;
    for (const auto& contents : data_->shared_array_buffer_contents()) {
      Local<SharedArrayBuffer> shared_array_buffer = SharedArrayBuffer::New(
          isolate_, contents.Data(), contents.ByteLength());
      deserializer_.TransferSharedArrayBuffer(index++, shared_array_buffer);
    }

    return deserializer_.ReadValue(context);
  }

 private:
  Isolate* isolate_;
  ValueDeserializer deserializer_;
  std::unique_ptr<SerializationData> data_;

  DISALLOW_COPY_AND_ASSIGN(Deserializer);
};

WorkerSerialization::WorkerSerialization(ArrayBuffer::Allocator*
    array_buffer_allocator)
    : array_buffer_allocator_(array_buffer_allocator) {}


SerializationData* WorkerSerialization::SerializeValue(Isolate* isolate,
    Local<Value> value) {
  bool ok;
  Local<Value> transfer = Local<Value>::Cast(Undefined(isolate));
  Local<Context> context = isolate->GetCurrentContext();
  Serializer serializer(isolate);
  if (serializer.WriteValue(context, value, transfer).To(&ok)) {
    std::unique_ptr<SerializationData> data = serializer.Release();
    base::LockGuard<base::Mutex> lock_guard(&mutex_);
    data->AppendExternalizedContentsTo(&externalized_contents_);
    return data.release();
  }
  return nullptr;
}


SerializationData* WorkerSerialization::SerializeValue(Isolate* isolate,
    Local<Value> value, Local<Array> transfer) {
  bool ok;
  Local<Context> context = isolate->GetCurrentContext();
  Serializer serializer(isolate);
  if (serializer.WriteValue(context, value, transfer).To(&ok)) {
    std::unique_ptr<SerializationData> data = serializer.Release();
    base::LockGuard<base::Mutex> lock_guard(&mutex_);
    data->AppendExternalizedContentsTo(&externalized_contents_);
    return data.release();
  }
  return nullptr;
}


MaybeLocal<Value> WorkerSerialization::DeserializeValue(Isolate* isolate,
    SerializationData* data) {
  Local<Value> value;
  Local<Context> context = isolate->GetCurrentContext();
  std::unique_ptr<SerializationData> out_data(data);
  Deserializer deserializer(isolate, std::move(out_data));
  MaybeLocal<Value> result = deserializer.ReadValue(context);
  return result;
}


void WorkerSerialization::CleanupExternalizedSharedContents() {
  // Now that all workers are terminated, we can re-enable Worker creation.
  base::LockGuard<base::Mutex> lock_guard(&mutex_);
  for (const auto& val : externalized_contents_) {
    val.FreeData(array_buffer_allocator_);
  }
  externalized_contents_.clear();
}


void WorkerSerialization::CleanupSerializationData(SerializationData* in_data) {
  if (in_data == nullptr) return;
  delete in_data;
}
}  // namespace worker
}  // namespace v8
