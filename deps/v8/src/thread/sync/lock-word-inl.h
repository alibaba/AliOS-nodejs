
// This file is come from Android Runtime VM.
// https://source.android.com/source/downloading.html

#ifndef THREAD_LOCK_WORD_INL_H_
#define THREAD_LOCK_WORD_INL_H_

#include "src/thread/sync/monitor.h"
#include "src/thread/vmthread-inl.h"

namespace v8 {
namespace internal {
namespace sync {

inline LockWord::LockWord(Monitor* mon, const int state)
  : value_(HeapObjectTagField::encode(kSmiTag) |
           StateField::encode(state) |
           MonitorIdField::encode(mon->GetMonitorId())) {
#ifndef __LP64__
  DCHECK((reinterpret_cast<intptr_t>(mon) & kMonitorIdAlignmentMask) == 0);
#endif
  DCHECK_EQ(FatLockMonitor(VMThread::Current()), mon);
  DCHECK_LE(mon->GetMonitorId(), static_cast<uint32_t>(kMaxMonitorId));
  DCHECK_EQ(HeapObjectTagField::decode(value_), kSmiTag);
}

inline Monitor* LockWord::FatLockMonitor(VMThread* self) const {
  DCHECK(GetState()== kFatMode ||
          GetState() == kFatLocked);
  MonitorId mon_id = MonitorIdField::decode(value_);
  return MonitorPool::MonitorFromMonitorId(self, mon_id);
}


inline void LockWord::Print(std::ostream& os, uint32_t monitor) {
  VMThread* self = VMThread::Current();
  LockWord lw(monitor);
  switch (lw.GetState()) {
    case LockWord::kThinLocked: {
      break;
      os << "\n - monitor: thin"
         << "\n     - lock_owner: " << lw.ThinLockOwner()
         << "\n     - lock-count: " << lw.ThinLockCount()
         << "\n     - self-thread-id:" << self->thread_id();
    }
    case LockWord::kFatMode:
    case LockWord::kFatLocked: {
      os << "\n - monitor: fat";
      Monitor* mon = lw.FatLockMonitor(self);
      mon->Print(os);
      break;
    }
    case LockWord::kUnlocked: {
      os << "\n - monitor: unlocked";
      break;
    }
    default: {
      os << "\n - monitor: Invalid monitor state 3";
      break;
    }
  }
}

}  // namespace sync
}  // namespace internal
}  // namespace v8
#endif  // THREAD_LOCK_WORD_INL_H_
