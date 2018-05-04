
// The source is based on Android Runtime VM.
// https://source.android.com/source/downloading.html

#ifndef THREAD_MUTEX_H_
#define THREAD_MUTEX_H_

#include <pthread.h>
#include <stdint.h>

#include <iosfwd>
#include <string>
#include "src/base/macros.h"
#include "src/globals.h"
#include "src/thread/sync/atomic.h"
#include "src/base/platform/time.h"

#if defined(__APPLE__)
#define V8_USE_FUTEXES 0
#else
#define V8_USE_FUTEXES 1
#endif

// Currently Darwin doesn't support locks with timeouts.
#if !defined(__APPLE__)
#define HAVE_TIMED_RWLOCK 1
#else
#define HAVE_TIMED_RWLOCK 0
#endif

#define LOCKABLE
#define SCOPED_LOCKABLE
#define EXCLUSIVE_LOCK_FUNCTION(...)
#define UNLOCK_FUNCTION(...)
#define EXCLUSIVE_TRYLOCK_FUNCTION(...)
#define SHARED_LOCK_FUNCTION(...)
#define NO_THREAD_SAFETY_ANALYSIS

namespace v8 {
namespace internal {
class VMThread;
namespace sync {

class LOCKABLE ReaderWriterMutex;
class ScopedContentionRecorder;

// LockLevel is used to impose a lock hierarchy [1] where
// acquisition of a Mutex at a higher or
// equal level to a lock a thread holds is invalid. The lock hierarchy
// achieves a cycle free
// partial ordering and thereby cause deadlock situations to fail checks.
//
// [1] http://www.drdobbs.com/parallel/
//         use-lock-hierarchies-to-avoid-deadlock/204801163
enum LockLevel {
  kDefaultMutexLevel = 0,
  kMutatorLock,
  kLockLevelCount  // Must come last.
};
// std::ostream& operator<<(std::ostream& os, const LockLevel& rhs);

#if DEBUG
const bool kDebugLocking = 1;
const bool kLogLockContentions = true;
const size_t kAllMutexDataSize = 1;
#else
const bool kDebugLocking = 0;
const bool kLogLockContentions = false;
#endif

const size_t kContentionLogSize = 4;


// Base class for all Mutex implementations
class BaseMutex {
 public:
  const char* GetName() const {
    return name_;
  }

  virtual bool IsMutex() const { return false; }
  virtual bool IsReaderWriterMutex() const { return false; }

  virtual void Dump(std::ostream& os) = 0;

  static void DumpAll(std::ostream& os);

 protected:
  friend class ConditionVariable;

  BaseMutex(const char* name, LockLevel level);
  virtual ~BaseMutex();
  void RegisterAsLocked(VMThread* self);
  void RegisterAsUnlocked(VMThread* self);
  void CheckSafeToWait(VMThread* self);

  friend class ScopedContentionRecorder;
  friend class v8::internal::VMThread;

  void RecordContention(uint64_t blocked_tid, uint64_t owner_tid,
                        uint64_t nano_time_blocked);
  void DumpContention(std::ostream& os);

  const LockLevel level_;  // Support for lock hierarchy.
  const char* const name_;

  // A log entry that records contention but makes no guarantee
  // that either tid will be held live.
  struct ContentionLogEntry {
    ContentionLogEntry() : blocked_tid(0), owner_tid(0) {}
    uint64_t blocked_tid;
    uint64_t owner_tid;
    AtomicInteger count;
  };
  struct ContentionLogData {
    ContentionLogEntry contention_log[kContentionLogSize];
    // The next entry in the contention log to be updated.
    // Value ranges from 0 to kContentionLogSize - 1.
    AtomicInteger cur_content_log_entry;
    // Number of times the Mutex has been contended.
    AtomicInteger contention_count;
    // Sum of time waited by all contenders in ns.
    Atomic<size_t> wait_time;
    void AddToWaitTime(size_t value);
    ContentionLogData() : wait_time(0) {}
  };

#if DEBUG
  ContentionLogData contention_log_data_[1];
#endif

 public:
  bool HasEverContended() {
#if DEBUG
    return contention_log_data_->
               contention_count.LoadSequentiallyConsistent() > 0;
#else
    return false;
#endif
  }
};


// A Mutex is used to achieve mutual exclusion between threads.
// A Mutex can be used to gain
// exclusive access to what it guards. A Mutex can be in one of two states:
// - Free - not owned by any thread,
// - Exclusive - owned by a single thread.
//
// The effect of locking and unlocking operations on the state is:
// State     | ExclusiveLock | ExclusiveUnlock
// -------------------------------------------
// Free      | Exclusive     | error
// Exclusive | Block*        | Free
// * Mutex is not reentrant and so an attempt to ExclusiveLock on the
//   same thread will result in
//   an error. Being non-reentrant simplifies Waiting on ConditionVariables.
// std::ostream& operator<<(std::ostream& os, const Mutex& mu);
class LOCKABLE Mutex : public BaseMutex {
 public:
  explicit Mutex(const char* name,
                 LockLevel level = kDefaultMutexLevel,
                 bool recursive = false);
  ~Mutex();

  virtual bool IsMutex() const { return true; }

  // Block until mutex is free then acquire exclusive access.
  void ExclusiveLock(VMThread* self) EXCLUSIVE_LOCK_FUNCTION();
  void Lock(VMThread* self) EXCLUSIVE_LOCK_FUNCTION() {  ExclusiveLock(self); }

  // Returns true if acquires exclusive access, false otherwise.
  bool ExclusiveTryLock(VMThread* self) EXCLUSIVE_TRYLOCK_FUNCTION(true);
  bool TryLock(VMThread* self) EXCLUSIVE_TRYLOCK_FUNCTION(true) {
    return ExclusiveTryLock(self);
  }

  // Release exclusive access.
  void ExclusiveUnlock(VMThread* self) UNLOCK_FUNCTION();
  void Unlock(VMThread* self) UNLOCK_FUNCTION() {  ExclusiveUnlock(self); }

  // Is the current thread the exclusive holder of the Mutex.
  bool IsExclusiveHeld(VMThread* self);

  // Assert that the Mutex is exclusively held by the current thread.
  void AssertExclusiveHeld(VMThread* self) {
    if (kDebugLocking) {
      CHECK(IsExclusiveHeld(self));
    }
  }
  void AssertHeld(VMThread* self) { AssertExclusiveHeld(self); }

  // Assert that the Mutex is not held by the current thread.
  void AssertNotHeldExclusive(VMThread* self) {
    if (kDebugLocking) {
      CHECK(!IsExclusiveHeld(self));
    }
  }
  void AssertNotHeld(VMThread* self) { AssertNotHeldExclusive(self); }

  // Id associated with exclusive owner. No memory ordering semantics
  // if called from a thread other than the owner.
  uint64_t GetExclusiveOwnerTid();

  // Returns how many times this Mutex has been locked,
  // it is better to use AssertHeld/NotHeld.
  unsigned int GetDepth() {
    return recursion_count_;
  }

  virtual void Dump(std::ostream& os);

 private:
#if V8_USE_FUTEXES
  // 0 is unheld, 1 is held.
  AtomicInteger state_;
  // Exclusive owner.
  volatile uint64_t exclusive_owner_;
  // Number of waiting contenders.
  AtomicInteger num_contenders_;
#else
  pthread_mutex_t mutex_;
  volatile uint64_t exclusive_owner_;  // Guarded by mutex_.
#endif
  const bool recursive_;  // Can the lock be recursively held?
  unsigned int recursion_count_;
  friend class ConditionVariable;
  DISALLOW_COPY_AND_ASSIGN(Mutex);
};


// A ReaderWriterMutex is used to achieve mutual exclusion between threads,
// similar to a Mutex.
// Unlike a Mutex a ReaderWriterMutex can be used to gain exclusive (writer)
// or shared (reader)
// access to what it guards. A flaw in relation to a Mutex is that it cannot be
// used with a
// condition variable. A ReaderWriterMutex can be in one of three states:
// - Free - not owned by any thread,
// - Exclusive - owned by a single thread,
// - Shared(n) - shared amongst n threads.
//
// The effect of locking and unlocking operations on the state is:
//
// State     | ExclusiveLock | ExclusiveUnlock | SharedLock       | SharedUnlock
// ----------------------------------------------------------------------------
// Free      | Exclusive     | error           | SharedLock(1)    | error
// Exclusive | Block         | Free            | Block            | error
// Shared(n) | Block         | error           | SharedLock(n+1)* | Shared(n-1)
//                                                                  or Free
// * for large values of n the SharedLock may block.
// std::ostream& operator<<(std::ostream& os, const ReaderWriterMutex& mu);
class LOCKABLE ReaderWriterMutex : public BaseMutex {
 public:
  explicit ReaderWriterMutex(const char* name,
                             LockLevel level = kDefaultMutexLevel);
  ~ReaderWriterMutex();

  virtual bool IsReaderWriterMutex() const { return true; }

  // Block until ReaderWriterMutex is free then acquire exclusive access.
  void ExclusiveLock(VMThread* self) EXCLUSIVE_LOCK_FUNCTION();
  void WriterLock(VMThread* self) EXCLUSIVE_LOCK_FUNCTION() {
    ExclusiveLock(self);
  }

  // Release exclusive access.
  void ExclusiveUnlock(VMThread* self) UNLOCK_FUNCTION();
  void WriterUnlock(VMThread* self) UNLOCK_FUNCTION() {
    ExclusiveUnlock(self);
  }

  // Block until ReaderWriterMutex is free and acquire exclusive access.
  // Returns true on success or false if timeout is reached.
#if HAVE_TIMED_RWLOCK
  bool ExclusiveLockWithTimeout(VMThread* self, const base::TimeDelta& rel_time)
      EXCLUSIVE_TRYLOCK_FUNCTION(true);
#endif

  // Block until ReaderWriterMutex is shared or free
  // then acquire a share on the access.
  void SharedLock(VMThread* self) SHARED_LOCK_FUNCTION();
  void ReaderLock(VMThread* self) SHARED_LOCK_FUNCTION() { SharedLock(self); }

  // Try to acquire share of ReaderWriterMutex.
  bool SharedTryLock(VMThread* self) EXCLUSIVE_TRYLOCK_FUNCTION(true);

  // Release a share of the access.
  void SharedUnlock(VMThread* self) UNLOCK_FUNCTION();
  void ReaderUnlock(VMThread* self) UNLOCK_FUNCTION() { SharedUnlock(self); }

  // Is the current thread the exclusive holder of the ReaderWriterMutex.
  bool IsExclusiveHeld(VMThread* self);

  // Assert the current thread has exclusive access to the ReaderWriterMutex.
  void AssertExclusiveHeld(VMThread* self) {
    if (kDebugLocking) {
      CHECK(IsExclusiveHeld(self));
    }
  }
  void AssertWriterHeld(VMThread* self) { AssertExclusiveHeld(self); }

  // Assert the current thread doesn't have
  // exclusive access to the ReaderWriterMutex.
  void AssertNotExclusiveHeld(VMThread* self) {
    if (kDebugLocking) {
      CHECK(!IsExclusiveHeld(self));
    }
  }
  void AssertNotWriterHeld(VMThread* self) { AssertNotExclusiveHeld(self); }

  // Is the current thread a shared holder of the ReaderWriterMutex.
  bool IsSharedHeld(VMThread* self);

  // Assert the current thread has shared access to the ReaderWriterMutex.
  void AssertSharedHeld(VMThread* self) {
    if (kDebugLocking) {
      // TODO: we can only assert this well when self != null.
      CHECK(IsSharedHeld(self) || self == nullptr);
    }
  }
  void AssertReaderHeld(VMThread* self) { AssertSharedHeld(self); }

  // Assert the current thread doesn't hold this ReaderWriterMutex
  // either in shared or exclusive mode.
  void AssertNotHeld(VMThread* self) {
    if (kDebugLocking) {
      CHECK(!IsSharedHeld(self));
    }
  }

  // Id associated with exclusive owner. No memory ordering semantics
  // if called from a thread other than the owner.
  uint64_t GetExclusiveOwnerTid();

  virtual void Dump(std::ostream& os);

 private:
#if V8_USE_FUTEXES
  // Out-of-inline path for handling contention for a SharedLock.
  void HandleSharedLockContention(VMThread* self, int32_t cur_state);

  // -1 implies held exclusive, +ve shared held by state_ many owners.
  AtomicInteger state_;
  // Exclusive owner. Modification guarded by this mutex.
  volatile uint64_t exclusive_owner_;
  // Number of contenders waiting for a reader share.
  AtomicInteger num_pending_readers_;
  // Number of contenders waiting to be the writer.
  AtomicInteger num_pending_writers_;
#else
  pthread_rwlock_t rwlock_;
  volatile uint64_t exclusive_owner_;  // Guarded by rwlock_.
#endif
  DISALLOW_COPY_AND_ASSIGN(ReaderWriterMutex);
};


// ConditionVariables allow threads to queue and sleep. VMThreads may
// then be resumed individually (Signal) or all at once (Broadcast).
class ConditionVariable {
 public:
  explicit ConditionVariable(const char* name, Mutex& mutex);
  ~ConditionVariable();

  void Broadcast(VMThread* self);
  void Signal(VMThread* self);
  // TODO: No thread safety analysis on Wait and TimedWait as they call mutex
  // operations via their pointer copy, thereby defeating annotalysis.
  void Wait(VMThread* self) NO_THREAD_SAFETY_ANALYSIS;
  bool TimedWait(VMThread* self, const base::TimeDelta& rel_time)
      NO_THREAD_SAFETY_ANALYSIS;
  // Variant of Wait that should be used with caution. Doesn't validate
  // that no mutexes are held when waiting.
  // TODO: remove this.
  void WaitHoldingLocks(VMThread* self) NO_THREAD_SAFETY_ANALYSIS;

 private:
  const char* const name_;
  // The Mutex being used by waiters. It is an error to mix condition
  // variables between different Mutexes.
  Mutex& guard_;
#if V8_USE_FUTEXES
  // A counter that is modified by signals and broadcasts. This ensures that
  // when a waiter gives up
  // their Mutex and another thread takes it and signals, the waiting thread
  // observes that sequence_
  // changed and doesn't enter the wait. Modified while holding guard_,
  // but is read by futex wait
  // without guard_ held.
  AtomicInteger sequence_;
  // Number of threads that have come into to wait, not the length of the
  // waiters on the futex as waiters may have been requeued onto guard_.
  // Guarded by guard_.
  volatile int32_t num_waiters_;
#else
  pthread_cond_t cond_;
#endif
  DISALLOW_COPY_AND_ASSIGN(ConditionVariable);
};

// Scoped locker/unlocker for a regular Mutex that acquires mu upon
// construction and releases it upon destruction.
class SCOPED_LOCKABLE MutexLock {
 public:
  explicit MutexLock(VMThread* self, Mutex& mu) EXCLUSIVE_LOCK_FUNCTION(mu)
    : self_(self), mu_(mu) {
    mu_.ExclusiveLock(self_);
  }

  ~MutexLock() UNLOCK_FUNCTION() {
    mu_.ExclusiveUnlock(self_);
  }

 private:
  VMThread* const self_;
  Mutex& mu_;
  DISALLOW_COPY_AND_ASSIGN(MutexLock);
};

class SCOPED_LOCKABLE MutexUnLock {
 public:
  explicit MutexUnLock(VMThread* self, Mutex& mu) UNLOCK_FUNCTION()
    : self_(self), mu_(mu) {
    mu_.ExclusiveUnlock(self_);
  }

  ~MutexUnLock()  EXCLUSIVE_LOCK_FUNCTION(mu_) {
    mu_.ExclusiveLock(self_);
  }

 private:
  VMThread* const self_;
  Mutex& mu_;
  DISALLOW_COPY_AND_ASSIGN(MutexUnLock);
};

// Catch bug where variable name is omitted. "MutexLock (lock);"
// instead of "MutexLock mu(lock)".
#define MutexLock(x) static_assert(0, \
                         "MutexLock declaration missing variable name")

// Scoped locker/unlocker for a ReaderWriterMutex that acquires read access
// to mu upon construction and releases it upon destruction.
class SCOPED_LOCKABLE ReaderMutexLock {
 public:
  explicit ReaderMutexLock(VMThread* self, ReaderWriterMutex& mu)
      EXCLUSIVE_LOCK_FUNCTION(mu) :
      self_(self), mu_(mu) {
    mu_.SharedLock(self_);
  }

  ~ReaderMutexLock() UNLOCK_FUNCTION() {
    mu_.SharedUnlock(self_);
  }

 private:
  VMThread* const self_;
  ReaderWriterMutex& mu_;
  DISALLOW_COPY_AND_ASSIGN(ReaderMutexLock);
};
// Catch bug where variable name is omitted.
// "ReaderMutexLock (lock);" instead of "ReaderMutexLock mu(lock)".
#define ReaderMutexLock(x) STATIC_ASSERT(0, \
                      "ReaderMutexLock declaration missing variable name")

// Scoped locker/unlocker for a ReaderWriterMutex that acquires write
// access to mu upon construction and releases it upon destruction.
class SCOPED_LOCKABLE WriterMutexLock {
 public:
  explicit WriterMutexLock(VMThread* self, ReaderWriterMutex& mu)
      EXCLUSIVE_LOCK_FUNCTION(mu) :
      self_(self), mu_(mu) {
    mu_.ExclusiveLock(self_);
  }

  ~WriterMutexLock() UNLOCK_FUNCTION() {
    mu_.ExclusiveUnlock(self_);
  }

 private:
  VMThread* const self_;
  ReaderWriterMutex& mu_;
  DISALLOW_COPY_AND_ASSIGN(WriterMutexLock);
};
// Catch bug where variable name is omitted.
// "WriterMutexLock (lock);" instead of "WriterMutexLock mu(lock)".
#define WriterMutexLock(x) STATIC_ASSERT(0, \
    "WriterMutexLock declaration missing variable name")

// Global mutexes corresponding to the levels above.
class Locks {
 public:
  static void Init();
  static void InitConditions() NO_THREAD_SAFETY_ANALYSIS;
  // Condition variables.
  // Guards allocation entrypoint instrumenting.
  // static Mutex* xxx_lock_;
};

class SCOPED_LOCKABLE SafepointMutexLock {
  public:
    explicit SafepointMutexLock(VMThread* self, Mutex& mu)
        EXCLUSIVE_LOCK_FUNCTION(mu);

    ~SafepointMutexLock() UNLOCK_FUNCTION() {
      if (should_lock_) mu_.ExclusiveUnlock(self_);
    }

  private:
    VMThread* const self_;
    Mutex& mu_;
    bool should_lock_;
    DISALLOW_COPY_AND_ASSIGN(SafepointMutexLock);
};

}  // namespace sync
}  // namespace internal
}  // namespace v8

#endif  // THREAD_MUTEX_H_
