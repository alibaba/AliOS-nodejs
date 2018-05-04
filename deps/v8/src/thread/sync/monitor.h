
// The source is based on Android Runtime VM.
// https://source.android.com/source/downloading.html

#ifndef THREAD_MONITOR_H_
#define THREAD_MONITOR_H_

#include "src/thread/sync/mutex.h"
#include "src/thread/sync/lock-word.h"
#include "src/zone/zone-containers.h"
#include "src/zone/zone.h"

namespace v8 {
namespace internal {
class VMThread;
class VMThreadList;

namespace sync {

typedef uint32_t MonitorId;


class Monitor {
 public:
  const static size_t kDefaultMaxSpinsBeforeThinLockInflation = 50;
  //--------------------------------------------------------------
  template<class Object>
  static inline void Lock(VMThread* self, Handle<Object> obj);
  template<class Object>
  static inline bool Unlock(VMThread* self, Handle<Object> obj);
  template<class Object>
  static inline void InflateThinLocked(VMThread* self,
                                Handle<Object> obj,
                                LockWord lock_word);
  template<class Object>
  static inline void Inflate(VMThread* self, VMThread* owner, Handle<Object> obj);

  template<class Object>
  static void inline Visit(VMThread* self, Object* obj);
  template<class Object>
  void inline Deflate(VMThread* self, Object* obj, bool is_locked);

  template<class Object>
  bool inline Install(VMThread* self, Handle<Object> obj);
  template<class Object>
  static bool Wait(VMThread* self, Handle<Object> obj, const base::TimeDelta& rel_time);
  template<class Object>
  static void Notify(VMThread* self, Handle<Object> obj, bool notify_all);
  template<class Object>
  void inline LockInternal(VMThread* self, Handle<Object> obj);
  template<class Object>
  bool inline UnlockInternal(VMThread* self, Handle<Object> obj);
  template<class Object>
  bool WaitInternal(VMThread* self, Handle<Object> obj, const base::TimeDelta& rel_time);

  static bool IsHeldMonitorLock(VMThread* self, uint32_t monitor);
  void NotifyInternal(VMThread* self);
  void NotifyAllInternal(VMThread* self);

  MonitorId GetMonitorId() const {
    return monitor_id_;
  }
  inline void Clear();

  static bool CasLockWordWeakSequentiallyConsistent(Address raw_addr,
      int32_t old_value, int32_t new_value) {
    AtomicInteger* atomic_addr = reinterpret_cast<AtomicInteger*>(raw_addr);
    return atomic_addr->CompareExchangeWeakSequentiallyConsistent(old_value,
                                                                  new_value);
  }

  static bool CasLockWordStrongSequentiallyConsistent(Address raw_addr,
      int32_t old_value, int32_t new_value) {
    AtomicInteger* atomic_addr = reinterpret_cast<AtomicInteger*>(raw_addr);
    return atomic_addr->CompareExchangeStrongSequentiallyConsistent(old_value,
                                                                  new_value);
  }

  void Print(std::ostream& os);

 private:
#ifndef __LP64__
  Monitor(VMThread* self, VMThread* owner);
#else
  Monitor(VMThread* self, VMThread* owner, MonitorId id);
#endif
  void AppendToWaitSet(VMThread* thread);
  void RemoveFromWaitSet(VMThread *thread);

  template<class Object>
  void CompeteLockWithIC(VMThread* self, Handle<Object> obj);

  Mutex monitor_lock_;
  ConditionVariable monitor_contenders_;
  VMThread* owner_;
  MonitorId monitor_id_;
  VMThread* wait_set_;
  uint32_t num_waiters_;
  uint32_t lock_count_;
  bool is_marked_;
#ifdef __LP64__
  // Free list for monitor pool.
  Monitor* next_free_;
#endif

  friend class MonitorPool;
  friend class MonitorList;
};


class MonitorList {
 public:
  MonitorList(VMThreadList* thread_list);
  ~MonitorList();

  static void FreeMonitors();

  void Add(Monitor* m);

  typedef ZoneLinkedList<Monitor*> Monitors;
 private:
  Mutex monitors_lock_;
  Zone zone_;
  ZoneLinkedList<Monitor*> monitors_;
  VMThreadList* thread_list_;
};


template<class Object>
class MonitorLockScope {
 public:
  explicit MonitorLockScope(VMThread* self, Handle<Object> obj) :
    self_(self), obj_(obj) {
    Monitor::Lock(self_, obj_);
  }

  ~MonitorLockScope() {
    Monitor::Unlock(self_, obj_);
  }

 private:
  VMThread* const self_;
  Handle<Object> obj_;
  DISALLOW_COPY_AND_ASSIGN(MonitorLockScope);
};


class RWMonitor {
 public:
   static const int kSmiTagShift = 1;
   static const int kDefaultUnlocked = 1;
   enum LockType {
    RLOCK,
    WLOCK,
    RWLOCK,
    NO_LOCK
  };

  enum StateMode {
    TRANSITION,
    NO_TRANSITION
  };
  static inline int32_t DefaultValueTagged() {
    return kDefaultUnlocked << kSmiTagShift;
  }
  static inline uint32_t Encode(uint32_t value) {
    return value << kSmiTagShift;
  }
  static inline uint32_t Decode(uint32_t value) {
    return value >> kSmiTagShift;
  }
  template<class Object>
  static bool IsLocked(Handle<Object> obj,
                       LockType type = RWLOCK);
  template<class Object>
  static bool Lock(VMThread* self,
                   Handle<Object> obj,
                   LockType lock_type,
                   StateMode mode = TRANSITION);

  template<class Object>
  static void Unlock(Handle<Object> obj, LockType lock_type);
};


template<class Object>
class RWMonitorLockScope {
 public:
  explicit RWMonitorLockScope(
      VMThread* self, Handle<Object> obj, RWMonitor::LockType type,
      RWMonitor::StateMode mode = sync::RWMonitor::TRANSITION)
   : self_(self),
     obj_(obj),
     type_(type),
     mode_(mode) {
     has_locked_ = RWMonitor::Lock(self_, obj_, type_, mode_);
  }

  ~RWMonitorLockScope() {
    if (has_locked_) {
      RWMonitor::Unlock(obj_, type_);
      has_locked_ = false;
    }
  }

  void UnlockScope() {
    if (has_locked_) {
      RWMonitor::Unlock(obj_, type_);
      has_locked_ = false;
    }
  }

 private:
  VMThread* const self_;
  Handle<Object> obj_;
  RWMonitor::LockType type_;
  RWMonitor::StateMode mode_;
  bool has_locked_;
  DISALLOW_COPY_AND_ASSIGN(RWMonitorLockScope);
};

// Abstraction to keep monitors small enough to fit in a lock word (32bits).
// On 32bit systems the monitor id loses the alignment bits of the Monitor*.
class MonitorPool {
 public:
  static MonitorPool* Create() {
#ifndef __LP64__
    return nullptr;
#else
    return new MonitorPool();
#endif
  }

  static Monitor* CreateMonitor(VMThread* self, VMThread* owner);
  static void ReleaseMonitor(VMThread* self, Monitor* monitor);
  static Monitor* MonitorFromMonitorId(VMThread* self, MonitorId mon_id);
  static MonitorId MonitorIdFromMonitor(Monitor* mon);
  static MonitorId ComputeMonitorId(Monitor* mon, VMThread* self);
  static MonitorPool* monitor_pool(VMThread* self);
  static void ReleaseMonitors(VMThreadList* thread_list,
                              VMThread* self,
                              MonitorList::Monitors* monitors);

  ~MonitorPool() {
#ifdef __LP64__
    FreeInternal();
#endif
  }

 private:
#ifdef __LP64__
  // When we create a monitor pool, threads have not been initialized, yet, so ignore thread-safety
  // analysis.
  MonitorPool();

  void AllocateChunk();

  // Release all chunks and metadata. This is done on shutdown, where threads have been destroyed,
  // so ignore thead-safety analysis.
  void FreeInternal();

  Monitor* CreateMonitorInPool(VMThread* self, VMThread* owner);

  void ReleaseMonitorToPool(VMThread* self, Monitor* monitor);
  void ReleaseMonitorsToPool(VMThread* self, MonitorList::Monitors* monitors);

  // Note: This is safe as we do not ever move chunks.  All needed entries in the monitor_chunks_
  // data structure are read-only once we get here.  Updates happen-before this call because
  // the lock word was stored with release semantics and we read it with acquire semantics to
  // retrieve the id.
  Monitor* LookupMonitor(MonitorId mon_id) {
    size_t offset = MonitorIdToOffset(mon_id);
    size_t index = offset / kChunkSize;
    size_t top_index = index / kMaxListSize;
    size_t list_index = index % kMaxListSize;
    size_t offset_in_chunk = offset % kChunkSize;
    uintptr_t base = monitor_chunks_[top_index][list_index];
    return reinterpret_cast<Monitor*>(base + offset_in_chunk);
  }

  static bool IsInChunk(uintptr_t base_addr, Monitor* mon) {
    uintptr_t mon_ptr = reinterpret_cast<uintptr_t>(mon);
    return base_addr <= mon_ptr && (mon_ptr - base_addr < kChunkSize);
  }

  MonitorId ComputeMonitorIdInPool(Monitor* mon, VMThread* self) {
    MutexLock mu(self, allocated_monitor_ids_lock_);
    for (size_t i = 0; i <= current_chunk_list_index_; ++i) {
      for (size_t j = 0; j < ChunkListCapacity(i); ++j) {
        if (j >= num_chunks_ && i == current_chunk_list_index_) {
          break;
        }
        uintptr_t chunk_addr = monitor_chunks_[i][j];
        if (IsInChunk(chunk_addr, mon)) {
          return OffsetToMonitorId(
              reinterpret_cast<uintptr_t>(mon) - chunk_addr
              + i * (kMaxListSize * kChunkSize) + j * kChunkSize);
        }
      }
    }
    V8_Fatal(__FILE__, __LINE__, "Did not find chunk that contains monitor.");
    return 0;
  }

  static const size_t MonitorIdToOffset(MonitorId id) {
    return id << 3;
  }

  static const MonitorId OffsetToMonitorId(size_t offset) {
    return static_cast<MonitorId>(offset >> 3);
  }

  static const size_t ChunkListCapacity(size_t index) {
    return kInitialChunkStorage << index;
  }

  // TODO: There are assumptions in the code that monitor addresses are 8B aligned (>>3).
  static const size_t kMonitorAlignment = 8;
  static const size_t kPageSize = 4 * KB;
  // Size of a monitor, rounded up to a multiple of alignment.
  static const size_t kAlignedMonitorSize = (sizeof(Monitor) + kMonitorAlignment - 1) &
                                                -kMonitorAlignment;
  // As close to a page as we can get seems a good start.
  static const size_t kChunkCapacity = kPageSize / kAlignedMonitorSize;
  // Chunk size that is referenced in the id. We can collapse this to the actually used storage
  // in a chunk, i.e., kChunkCapacity * kAlignedMonitorSize, but this will mean proper divisions.
  static const size_t kChunkSize = kPageSize;
  // The number of chunks of storage that can be referenced by the initial chunk list.
  // The total number of usable monitor chunks is typically 255 times this number, so it
  // should be large enough that we don't run out. We run out of address bits if it's > 512.
  // Currently we set it a bit smaller, to save half a page per process.  We make it tiny in
  // debug builds to catch growth errors. The only value we really expect to tune.
  static const size_t kInitialChunkStorage = 256U;
  // The number of lists, each containing pointers to storage chunks.
  static const size_t kMaxChunkLists = 8;  //  Dictated by 3 bit index. Don't increase above 8.
  static const size_t kMaxListSize = kInitialChunkStorage << (kMaxChunkLists - 1);
  // We lose 3 bits in monitor id due to 3 bit monitor_chunks_ index, and gain it back from
  // the 3 bit alignment constraint on monitors:

  // Array of pointers to lists (again arrays) of pointers to chunks containing monitors.
  // Zeroth entry points to a list (array) of kInitialChunkStorage pointers to chunks.
  // Each subsequent list as twice as large as the preceding one.
  // Monitor Ids are interpreted as follows:
  //     Top 3 bits (of 28): index into monitor_chunks_.
  //     Next 16 bits: index into the chunk list, i.e. monitor_chunks_[i].
  //     Last 9 bits: offset within chunk, expressed as multiple of kMonitorAlignment.
  // If we set kInitialChunkStorage to 512, this would allow us to use roughly 128K chunks of
  // monitors, which is 0.5GB of monitors.  With this maximum setting, the largest chunk list
  // contains 64K entries, and we make full use of the available index space. With a
  // kInitialChunkStorage value of 256, this is proportionately reduced to 0.25GB of monitors.
  // Updates to monitor_chunks_ are guarded by allocated_monitor_ids_lock_ .
  // No field in this entire data structure is ever updated once a monitor id whose lookup
  // requires it has been made visible to another thread.  Thus readers never race with
  // updates, in spite of the fact that they acquire no locks.
  uintptr_t* monitor_chunks_[kMaxChunkLists];  //  uintptr_t is really a Monitor* .
  // Highest currently used index in monitor_chunks_ . Used for newly allocated chunks.
  size_t current_chunk_list_index_;
  // Number of chunk pointers stored in monitor_chunks_[current_chunk_list_index_] so far.
  size_t num_chunks_;
  // After the initial allocation, this is always equal to
  // ChunkListCapacity(current_chunk_list_index_).
  size_t current_chunk_list_capacity_;

  Mutex allocated_monitor_ids_lock_;

  // Start of free list of monitors.
  // Note: these point to the right memory regions, but do *not* denote initialized objects.
  Monitor* first_free_;
#endif
};

}  // namespace sync
}  // namespace internal
}  // namespace v8

#endif  // THREAD_MONITOR_H_
