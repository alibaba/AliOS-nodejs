// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_SNAPSHOT_SERIALIZER_COMMON_H_
#define V8_SNAPSHOT_SERIALIZER_COMMON_H_

#include "src/address-map.h"
#include "src/base/bits.h"
#include "src/external-reference-table.h"
#include "src/globals.h"
#include "src/visitors.h"

namespace v8 {
namespace internal {

class Isolate;

class ExternalReferenceEncoder {
 public:
  explicit ExternalReferenceEncoder(Isolate* isolate);

  uint32_t Encode(Address key) const;

  const char* NameOfAddress(Isolate* isolate, Address address) const;

 private:
  AddressToIndexHashMap* map_;
#ifdef DEBUG
  ExternalReferenceTable* table_;
#endif  // DEBUG

  DISALLOW_COPY_AND_ASSIGN(ExternalReferenceEncoder);
};

class HotObjectsList {
 public:
  HotObjectsList() : index_(0) {
    for (int i = 0; i < kSize; i++) circular_queue_[i] = NULL;
  }

  void Add(HeapObject* object) {
    DCHECK(!AllowHeapAllocation::IsAllowed());
    circular_queue_[index_] = object;
    index_ = (index_ + 1) & kSizeMask;
  }

  HeapObject* Get(int index) {
    DCHECK(!AllowHeapAllocation::IsAllowed());
    DCHECK_NOT_NULL(circular_queue_[index]);
    return circular_queue_[index];
  }

  static const int kNotFound = -1;

  int Find(HeapObject* object) {
    DCHECK(!AllowHeapAllocation::IsAllowed());
    for (int i = 0; i < kSize; i++) {
      if (circular_queue_[i] == object) return i;
    }
    return kNotFound;
  }

  static const int kSize = 8;

 private:
  static_assert(base::bits::IsPowerOfTwo(kSize), "kSize must be power of two");
  static const int kSizeMask = kSize - 1;
  HeapObject* circular_queue_[kSize];
  int index_;

  DISALLOW_COPY_AND_ASSIGN(HotObjectsList);
};

const char* SerializerSpaceName(SerializerSpace space);
SerializerSpace SelectSerializerSpace(AllocationSpace alloction_space);
AllocationSpace SerializerSpaceToAllocationSpace(SerializerSpace space);
Sharability SerializerSpaceToSharability(SerializerSpace space);
// The Serializer/Deserializer class is a common superclass for Serializer and
// Deserializer which is used to store common constants and methods used by
// both.
class SerializerDeserializer : public RootVisitor {
 public:
  static void Iterate(Isolate* isolate, RootVisitor* visitor);

  // No reservation for large object space necessary.
  // We also handle map space differenly.
  STATIC_ASSERT(kSerializerMapSpace == kSerializerSharedCodeSpace + 1);
  static const int kNumberOfPreallocatedSpaces = kSerializerSharedCodeSpace + 1;
  static const int kNumberOfSpaces = NUMBER_OF_SERIALIZER_SPACE;
  static const int kNumberOfReservations = kSerializerSharedMapSpace + 1;

 protected:
  static bool CanBeDeferred(HeapObject* o);

  void RestoreExternalReferenceRedirectors(List<AccessorInfo*>* accessor_infos);

  // <YUNOS> changed: begin
  // ---------- byte code range 0x00..0x7f ----------
  // Byte codes in this range represent Where, HowToCode and WhereToPoint.
  // Where the pointed-to object can be found:
  // The static assert below will trigger when the number of preallocated spaces
  // changed. If that happens, update the bytecode ranges in the comments below.
  STATIC_ASSERT(8 == kNumberOfSpaces);
  enum Where {
    // 0x00..0x07  Allocate new object, in specified space.
    kNewObject = 0x00,
    // 0x08..0x0f  Reference to previous object from space.
    kBackref = 0x08,
    // 0x10..0x17  Reference to previous object from space after skip.
    kBackrefWithSkip = 0x10,

    // 0x18, 0x38        Root array item.
    kRootArray = 0x18,
    // 0x19, 0x59        Object in the partial snapshot cache.
    kPartialSnapshotCache = 0x19,
    // 0x1a, 0x3a        External reference referenced by id.
    kExternalReference = 0x1a,

    // 0x1b, 3b, 5b, 7b  Object provided in the attached list.
    kAttachedReference = 0x1b,
    // 0x1c, 5c, 7c      Builtin code referenced by index.
    kBuiltin = 0x1c,

    // 0x58, 0x78
    // 0x39, 0x79
    // 0x5a, 0x7a
    // 0x3c
    // 0x1d..0x1f  Misc, see below (incl. 0x3d..0x3f, 0x5d..0x5f, 0x7d..0x7f).
  };

  enum LoSpaceAttribute {
    NOT_EXECUTABLE_SHARED = 0, // 00
    NOT_EXECUTABLE_NOT_SHARED, // 01
    EXECUTABLE_SHARED,         // 10
    EXECUTABLE_NOT_SHARED,     // 11
  };
  static const int kSharabilityMask = 0x1;
  static const int kSharabilityShift = 0;
  static const int kExecutabilityMask = 0x2;
  static const int kExecutabilityShift = 1;

  static const int kWhereMask = 0x1f;
  static const int kSpaceMask = 7;
  STATIC_ASSERT(kNumberOfSpaces <= kSpaceMask + 1);

  // How to code the pointer to the object.
  enum HowToCode {
    // Straight pointer.
    kPlain = 0,
    // A pointer inlined in code. What this means depends on the architecture.
    kFromCode = 0x20
  };

  static const int kHowToCodeMask = 0x20;

  // Where to point within the object.
  enum WhereToPoint {
    // Points to start of object
    kStartOfObject = 0,
    // Points to instruction in code object or payload of cell.
    kInnerPointer = 0x40
  };

  static const int kWhereToPointMask = 0x40;

  // ---------- Misc ----------
  // Skip.
  static const int kSkip = 0x3d;
  // Do nothing, used for padding.
  static const int kNop = 0x5d;
  // Move to next reserved chunk.
  static const int kNextChunk = 0x7d;
  // Deferring object content.
  static const int kDeferred = 0x3e;
  // Alignment prefixes 0x1d..0x1f
  static const int kAlignmentPrefix = 0x1d;
  // A tag emitted at strategic points in the snapshot to delineate sections.
  // If the deserializer does not find these at the expected moments then it
  // is an indication that the snapshot and the VM do not fit together.
  // Examine the build process for architecture, version or configuration
  // mismatches.
  static const int kSynchronize = 0x5e;
  // Repeats of variable length.
  static const int kVariableRepeat = 0x7e;
  // Raw data of variable length.
  static const int kVariableRawData = 0x3f;
  // Internal reference encoded as offsets of pc and target from code entry.
  static const int kInternalReference = 0x5f;
  static const int kInternalReferenceEncoded = 0x7f;
  // Used to encode deoptimizer entry code.
  static const int kDeoptimizerEntryPlain = 0x58;
  static const int kDeoptimizerEntryFromCode = 0x78;
  // Used for embedder-provided serialization data for embedder fields.
  static const int kEmbedderFieldsData = 0x39;

  // 0x79 0x5a 0x7a 0x3c unused.

  // ---------- byte code range 0x80..0xff ----------
  // 8 hot (recently seen or back-referenced) objects with optional skip.
  static const int kNumberOfHotObjects = 8;
  STATIC_ASSERT(kNumberOfHotObjects == HotObjectsList::kSize);
  // 0xf0..0xf7
  static const int kHotObject = 0xf0;
  // 0xf8..0xff
  static const int kHotObjectWithSkip = 0xf8;
  static const int kHotObjectMask = 0x07;

  // First 32 root array items.
  static const int kNumberOfRootArrayConstants = 0x20;
  // 0x80..0x9f
  static const int kRootArrayConstants = 0x80;
  // 0xa0..0xbf
  static const int kRootArrayConstantsWithSkip = 0xa0;
  static const int kRootArrayConstantsMask = 0x1f;

  // 32 common raw data lengths.
  static const int kNumberOfFixedRawData = 0x20;
  // 0xc0..0xdf
  static const int kFixedRawData = 0xc0;
  static const int kOnePointerRawData = kFixedRawData;
  static const int kFixedRawDataStart = kFixedRawData - 1;

  // 16 repeats lengths.
  static const int kNumberOfFixedRepeat = 0x10;
  // 0xe0..0xef
  static const int kFixedRepeat = 0xe0;
  static const int kFixedRepeatStart = kFixedRepeat - 1;

  // ---------- special values ----------
  static const int kAnyOldSpace = -1;

  // Sentinel after a new object to indicate that double alignment is needed.
  static const int kDoubleAlignmentSentinel = 0;

  // ---------- member variable ----------
  HotObjectsList hot_objects_;
};

class SerializedData {
 public:
  class Reservation {
   public:
    explicit Reservation(uint32_t size)
        : reservation_(ChunkSizeBits::encode(size)) {}

    uint32_t chunk_size() const { return ChunkSizeBits::decode(reservation_); }
    bool is_last() const { return IsLastChunkBits::decode(reservation_); }

    void mark_as_last() { reservation_ |= IsLastChunkBits::encode(true); }

   private:
    uint32_t reservation_;
  };

  SerializedData(byte* data, int size)
      : data_(data), size_(size), owns_data_(false) {}
  SerializedData() : data_(NULL), size_(0), owns_data_(false) {}
  SerializedData(SerializedData&& other)
      : data_(other.data_), size_(other.size_), owns_data_(other.owns_data_) {
    // Ensure |other| will not attempt to destroy our data in destructor.
    other.owns_data_ = false;
  }

  ~SerializedData() {
    if (owns_data_) DeleteArray<byte>(data_);
  }

  uint32_t GetMagicNumber() const { return GetHeaderValue(kMagicNumberOffset); }
  uint32_t GetExtraReferences() const {
    return GetHeaderValue(kExtraExternalReferencesOffset);
  }

  class ChunkSizeBits : public BitField<uint32_t, 0, 31> {};
  class IsLastChunkBits : public BitField<bool, 31, 1> {};

  static uint32_t ComputeMagicNumber(ExternalReferenceTable* table) {
    uint32_t external_refs = table->size() - table->num_api_references();
    return 0xC0DE0000 ^ external_refs;
  }
  static uint32_t GetExtraReferences(ExternalReferenceTable* table) {
    return table->num_api_references();
  }

  static const int kMagicNumberOffset = 0;
  static const int kExtraExternalReferencesOffset =
      kMagicNumberOffset + kInt32Size;
  static const int kVersionHashOffset =
      kExtraExternalReferencesOffset + kInt32Size;

 protected:
  void SetHeaderValue(int offset, uint32_t value) {
    memcpy(data_ + offset, &value, sizeof(value));
  }

  uint32_t GetHeaderValue(int offset) const {
    uint32_t value;
    memcpy(&value, data_ + offset, sizeof(value));
    return value;
  }

  void AllocateData(int size);

  static uint32_t ComputeMagicNumber(Isolate* isolate) {
    return ComputeMagicNumber(ExternalReferenceTable::instance(isolate));
  }
  static uint32_t GetExtraReferences(Isolate* isolate) {
    return GetExtraReferences(ExternalReferenceTable::instance(isolate));
  }

  void SetMagicNumber(Isolate* isolate) {
    SetHeaderValue(kMagicNumberOffset, ComputeMagicNumber(isolate));
    SetHeaderValue(kExtraExternalReferencesOffset, GetExtraReferences(isolate));
  }

  byte* data_;
  int size_;
  bool owns_data_;

 private:
  DISALLOW_COPY_AND_ASSIGN(SerializedData);
};

}  // namespace internal
}  // namespace v8

#endif  // V8_SNAPSHOT_SERIALIZER_COMMON_H_
