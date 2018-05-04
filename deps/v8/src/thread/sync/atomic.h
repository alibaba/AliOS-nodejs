
// The source is based on Android Runtime VM.
// https://source.android.com/source/downloading.html

#ifndef THREAD_ATOMIC_H_
#define THREAD_ATOMIC_H_

#include <stdint.h>
#include <atomic>
#include <limits>

#include "src/base/macros.h"

namespace v8 {
namespace internal {
namespace sync {

inline void ThreadFenceForConstructor() {
#if defined(__aarch64__)
  __asm__ __volatile__("dmb ishst" : : : "memory");
#elif defined(__GLIBCXX__)
  __atomic_thread_fence(std::memory_order_release);
#else
  std::atomic_thread_fence(std::memory_order_release);
#endif
}

template<typename T>
class Atomic : public std::atomic<T> {
 public:
  Atomic<T>() : std::atomic<T>(0) { }

  explicit Atomic<T>(T value) : std::atomic<T>(value) { }

  // Load from memory without ordering or synchronization constraints.
  T LoadRelaxed() const {
    return this->load(std::memory_order_relaxed);
  }

  // Load from memory with a total ordering.
  T LoadSequentiallyConsistent() const {
    return this->load(std::memory_order_seq_cst);
  }

  // Load from memory with a acquire.
  T LoadAcquire() const {
    return this->load(std::memory_order_acquire);
  }

  // Store to memory without ordering or synchronization constraints.
  void StoreRelaxed(T desired) {
    this->store(desired, std::memory_order_relaxed);
  }

  // Store to memory with release ordering.
  void StoreRelease(T desired) {
    this->store(desired, std::memory_order_release);
  }

  // Store to memory with a total ordering.
  void StoreSequentiallyConsistent(T desired) {
    this->store(desired, std::memory_order_seq_cst);
  }

  // Atomically replace the value with desired value if it matches the
  // expected value. Participates in total ordering of atomic operations.
  bool CompareExchangeStrongSequentiallyConsistent(T expected_value,
                                                   T desired_value) {
    return this->compare_exchange_strong(expected_value,
                                         desired_value,
                                         std::memory_order_seq_cst);
  }

  // The same, except it may fail spuriously.
  bool CompareExchangeWeakSequentiallyConsistent(T expected_value,
                                                 T desired_value) {
    return this->compare_exchange_weak(expected_value,
                                       desired_value,
                                       std::memory_order_seq_cst);
  }

  // Atomically replace the value with desired value if it matches the expected
  // value. Doesn't imply ordering or synchronization constraints.
  bool CompareExchangeStrongRelaxed(T expected_value, T desired_value) {
    return this->compare_exchange_strong(expected_value,
                                         desired_value,
                                         std::memory_order_relaxed);
  }

  // The same, except it may fail spuriously.
  bool CompareExchangeWeakRelaxed(T expected_value, T desired_value) {
    return this->compare_exchange_weak(expected_value,
                                       desired_value,
                                       std::memory_order_relaxed);
  }

  // Atomically replace the value with desired value if it matches the
  // expected value. Prior writes made to other memory locations by the
  // thread that did the release become visible in this thread.
  bool CompareExchangeWeakAcquire(T expected_value, T desired_value) {
    return this->compare_exchange_weak(expected_value,
                                       desired_value,
                                       std::memory_order_acquire);
  }

  // Atomically replace the value with desired value if it matches the
  // expected value. prior writes to other memory locations become visible
  // to the threads that do a consume or an acquire on the same location.
  bool CompareExchangeWeakRelease(T expected_value, T desired_value) {
    return this->compare_exchange_weak(expected_value,
                                       desired_value,
                                       std::memory_order_release);
  }

  T FetchAndAddSequentiallyConsistent(const T value) {
    // Return old_value.
    return this->fetch_add(value, std::memory_order_seq_cst);
  }

  T FetchAndSubSequentiallyConsistent(const T value) {
    // Return old_value.
    return this->fetch_sub(value, std::memory_order_seq_cst);
  }

  T FetchAndOrSequentiallyConsistent(const T value) {
    // Return old_value.
    return this->fetch_or(value, std::memory_order_seq_cst);
  }

  T FetchAndAndSequentiallyConsistent(const T value) {
    // Return old_value.
    return this->fetch_and(value, std::memory_order_seq_cst);
  }

  volatile T* Address() {
    return reinterpret_cast<T*>(this);
  }

  static T MaxValue() {
    return std::numeric_limits<T>::max();
  }
};

typedef Atomic<int32_t> AtomicInteger;
typedef Atomic<void*> AtomicPtr;

STATIC_ASSERT(sizeof(AtomicInteger) == sizeof(int32_t));
STATIC_ASSERT(alignof(AtomicInteger) == alignof(int32_t));
STATIC_ASSERT(sizeof(Atomic<int64_t>) == sizeof(int64_t));

// Assert the alignment of 64-bit integers is 64-bit. This isn't true
// on certain 32-bit architectures (e.g. x86-32) but we know that 64-bit
// integers here are arranged to be 8-byte aligned.
#if defined(__LP64__)
  STATIC_ASSERT(alignof(Atomic<int64_t>) == alignof(int64_t));
#endif

}  // namespace sync
}  // namespace base
}  // namespace v8

#endif  // THREAD_ATOMIC_H
