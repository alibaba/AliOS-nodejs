
// The source is based on Android Runtime VM.
// https://source.android.com/source/downloading.html

#ifndef THREAD_LOCK_WORD_H_
#define THREAD_LOCK_WORD_H_

#include "include/v8.h"
#include "src/utils.h"
#include "src/base/logging.h"
#include "src/ostreams.h"

namespace v8 {
namespace internal {
class VMThread;

namespace sync {
class Monitor;

/*
 *  When the lock word is in the "thin" state and its bits
 *  are formatted as follows:
 *
 *  |3|3|22222222221|111111111000000|00|0|
 *  |1 |0 |98765432109|876543210987654|01|0|
 *  |b1|b2|lock count |thread id owner|00|0|
 *  b1: used by ObjectCheckScope
 *  b2: used by store ic check
 *
 *  When the lock word is in the "fat" state and its bits are
 *  formatted as follows:
 *
 *  |3322222222221111111111000000|00|0|
 *  |1098765432109876543210987654|10|0|
 *  | MonitorId                  |10|0|
 */
class LockWord {
 public:
  static const int kStateUnlocked = 0;
  static const int kStateThin = 1;
  static const int kStateFat = 2;
  static const int kStateFatLocked = 3;
  static const int kStateLockedMask = 1;

  static const int kHeapObjectTagSize = 1;
  static const int kStateSize = 2;
  static const int kThinLockOwnerSize = 16;
  static const int kICFlagSize = 1;
  static const int kObjectLockCheckSize = 1;
  static const int kThinLockCountSize =
      32 - kThinLockOwnerSize - kStateSize - kHeapObjectTagSize -
          kICFlagSize - kObjectLockCheckSize;
  static const int kMonitorIdSize = 29;
  static const int kMaxMonitorId = (1 << kMonitorIdSize) - 1;
  static const int kMonitorIdAlignmentShift = 32 - kMonitorIdSize;
  static const int kMonitorIdAlignmentMask =
      (1 << kMonitorIdAlignmentShift) - 1;
  static const int kThinLockMaxOwner = (1 << kThinLockOwnerSize) - 1;
  static const int kThinLockMaxCount = (1 << kThinLockCountSize) - 1;

  enum LockState {
    kUnlocked = 0,    // No lock owners.
    kThinLocked,  // Single uncontended owner.
    kFatMode,   // See associated monitor.
    kFatLocked, // the locked fat mode, add for storeIC
  };
  // Thin mode
  class HeapObjectTagField : public BitField<
      bool, 0, kHeapObjectTagSize> {};
  class StateField : public BitField<
      int, HeapObjectTagField::kNext, kStateSize> {};
  class ThinLockOwnerField : public BitField<
      int, StateField::kNext, kThinLockOwnerSize> {};
  class ThinLockCountField : public BitField<
      int, ThinLockOwnerField::kNext, kThinLockCountSize> {};
  class ICFlagField : public BitField<
      bool, ThinLockCountField::kNext, kICFlagSize> {};
  class ObjectLockCheckField : public BitField<
      bool, ICFlagField::kNext, kObjectLockCheckSize> {};
  // Fat mode
  class MonitorIdField : public BitField<
      int, StateField::kNext, kMonitorIdSize> {};
  STATIC_ASSERT(MonitorIdField::kNext <= 32);

  static LockWord FromThinLockId(uint32_t thread_id, uint32_t count) {
    CHECK_LE(thread_id, static_cast<uint32_t>(kThinLockMaxOwner));
    CHECK_LE(count, static_cast<uint32_t>(kThinLockMaxCount));
    return LockWord(HeapObjectTagField::encode(kSmiTag) |
                    StateField::encode(kStateThin) |
                    ThinLockOwnerField::encode(thread_id) |
                    ThinLockCountField::encode(count));
  }

  static LockWord FromThinLockIdChecked(uint32_t thread_id, uint32_t count) {
    CHECK_LE(thread_id, static_cast<uint32_t>(kThinLockMaxOwner));
    CHECK_LE(count, static_cast<uint32_t>(kThinLockMaxCount));
    return LockWord(HeapObjectTagField::encode(kSmiTag) |
                    StateField::encode(kStateThin) |
                    ThinLockOwnerField::encode(thread_id) |
                    ThinLockCountField::encode(count) |
                    ObjectLockCheckField::encode(true));
  }

  static uint32_t ICLockTestMask() {
    return kStateLockedMask << StateField::kShift;
  }

  static uint32_t ICUnlockMask() {
    return ~(kStateLockedMask << StateField::kShift);
  }

  static LockWord ICLockWordFromFatValue(uint32_t value) {
    return LockWord(value | ICLockTestMask());
  }

  static LockWord ICLockWordFromThinValue(uint32_t value) {
    return LockWord(value | StateField::encode(kThinLocked) |
                    ICFlagField::encode(true));
  }

  static uint32_t ICLockWord() {
    return LockWord(HeapObjectTagField::encode(kSmiTag) |
                    StateField::encode(kThinLocked) |
                    ICFlagField::encode(true)).GetValue();
  }

  static void Print(std::ostream& os, uint32_t monitor);

  static bool IsDefault(LockWord lw) {
    return LockWord().GetValue() == lw.GetValue();
  }

  static LockWord Default() {
    return LockWord();
  }

  static uint32_t DefaultValue() {
    return LockWord().GetValue();
  }

  LockState GetState() const {
    uint32_t internal_state = StateField::decode(value_);
    return static_cast<LockState>(internal_state);
  }

  uint32_t ThinLockOwner() const {
    DCHECK_EQ(GetState(), kThinLocked);
    return ThinLockOwnerField::decode(value_);
  }

  uint32_t ThinLockCount() const {
    DCHECK_EQ(GetState(), kThinLocked);
    return ThinLockCountField::decode(value_);
  }

  bool IsLockedByIC() const {
    return ICFlagField::decode(value_);
  }

  static LockWord ClearICLock(const LockWord lock_word) {
    LockState state = lock_word.GetState();
    uint32_t value = lock_word.GetValue();
    if (state == kThinLocked && lock_word.IsLockedByIC()) {
      value = DefaultValue();
    } else if (state == kFatLocked) {
      value = StateField::update(value, kFatMode);
    }
    return LockWord(value);
  }

  Monitor* FatLockMonitor(VMThread* self) const;

 private:
  LockWord(): value_(kSmiTag) {
    DCHECK_EQ(GetState(), kUnlocked);
    DCHECK_EQ(HeapObjectTagField::decode(value_), kSmiTag);
  }

  explicit LockWord(uint32_t val) : value_(val) {
    DCHECK_EQ(HeapObjectTagField::decode(value_), kSmiTag);
  }
  explicit LockWord(Monitor* mon, const int state = LockWord::kStateFat);

  bool operator==(const LockWord& rhs);

  uint32_t GetValue() const {
    return value_;
  }

  void SetValue(uint32_t val) {
    value_ = val;
  }

  bool GetObjectLockCheck() const {
    return ObjectLockCheckField::decode(value_);
  }

  uint32_t value_;
  friend class Monitor;
  friend class MacroAssembler;
};

STATIC_ASSERT(sizeof(LockWord) == sizeof(uint32_t));

}  // namespace sync
}  // namespace internal
}  // namespace v8
#endif  // THREAD_LOCK_WORD_H_
