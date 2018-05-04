
// The source is based on Android Runtime VM.
// https://source.android.com/source/downloading.html

#include "src/thread/sync/monitor-inl.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sched.h>
#endif

namespace v8 {
namespace internal {
namespace sync {

#ifndef __LP64__
Monitor::Monitor(VMThread* self, VMThread* owner)
    : monitor_lock_("monitor lock"),
      monitor_contenders_("monitor contenders", monitor_lock_),
      owner_(owner),
      monitor_id_(MonitorPool::ComputeMonitorId(this, self)),
      wait_set_(NULL),
      num_waiters_(0),
      lock_count_(0),
      is_marked_(false) {
    CHECK(owner == NULL || owner == self || owner->IsSuspended());
  }
#else
Monitor::Monitor(VMThread* self, VMThread* owner, MonitorId id)
    : monitor_lock_("monitor lock"),
      monitor_contenders_("monitor contenders", monitor_lock_),
      owner_(owner),
      monitor_id_(id),
      wait_set_(NULL),
      num_waiters_(0),
      lock_count_(0),
      is_marked_(false),
      next_free_(NULL) {
    CHECK(owner == NULL || owner == self || owner->IsSuspended());
  }
#endif


void Monitor::AppendToWaitSet(VMThread* thread) {
  DCHECK(owner_ == VMThread::Current());
  DCHECK(thread != NULL);
  DCHECK(thread->wait_next_vmthread() == NULL);
  if (wait_set_ == NULL) {
    wait_set_ = thread;
    return;
  }

  // push_back.
  VMThread* t = wait_set_;
  while (t->wait_next_vmthread() != NULL) {
    t = t->wait_next_vmthread();
  }
  t->set_wait_next_vmthread(thread);
}

void Monitor::RemoveFromWaitSet(VMThread *thread) {
  DCHECK(owner_ == VMThread::Current());
  DCHECK(thread != NULL);
  if (wait_set_ == NULL) {
    return;
  }
  if (wait_set_ == thread) {
    wait_set_ = thread->wait_next_vmthread();
    thread->set_wait_next_vmthread(NULL);
    return;
  }

  VMThread* t = wait_set_;
  while (t->wait_next_vmthread() != NULL) {
    if (t->wait_next_vmthread() == thread) {
      t->set_wait_next_vmthread(thread->wait_next_vmthread());
      thread->set_wait_next_vmthread(NULL);
      return;
    }
    t = t->wait_next_vmthread();
  }
}


void Monitor::NotifyInternal(VMThread* self) {
  DCHECK(self != NULL);
  MutexLock mu(self, monitor_lock_);
  // Signal the first waiting thread in the wait set.
  while (wait_set_ != NULL) {
    VMThread* thread = wait_set_;
    wait_set_ = thread->wait_next_vmthread();
    thread->set_wait_next_vmthread(NULL);

    // Check to see if the thread is still waiting.
    MutexLock wait_mu(self, *thread->wait_mutex());
    if (thread->wait_monitor() != NULL) {
      thread->wait_condition_variable()->Signal(self);
      return;
    }
  }
}


void Monitor::NotifyAllInternal(VMThread* self) {
  DCHECK(self != NULL);
  MutexLock mu(self, monitor_lock_);
  // Signal all threads in the wait set.
  while (wait_set_ != NULL) {
    VMThread* thread = wait_set_;
    wait_set_ = thread->wait_next_vmthread();
    thread->set_wait_next_vmthread(NULL);
    MutexLock wait_mu(self, *thread->wait_mutex());
    if (thread->wait_monitor() != NULL) {
      thread->wait_condition_variable()->Signal(self);
    }
  }
}


bool Monitor::IsHeldMonitorLock(VMThread* self, uint32_t monitor) {
  LockWord lock_word(monitor);
  switch (lock_word.GetState()) {
    case LockWord::kThinLocked:
      return lock_word.ThinLockOwner() == self->thread_id();
    case LockWord::kFatMode:
    case LockWord::kFatLocked: {
      Monitor* mon = lock_word.FatLockMonitor(self);
      return mon->owner_ == self;
    }
    case LockWord::kUnlocked:
    default:
      return false;
  }
}


void Monitor::Print(std::ostream& os) {
  VMThread* self = VMThread::Current();
  MutexLock mu(self, monitor_lock_);
  os << "\n     - fat monitor: "
     << "\n          - num_waiters: " << num_waiters_
     << "\n          - owner: " << (owner_ == NULL ? -1 : owner_->thread_id())
     << "\n          - lock_count: " << lock_count_;
}


MonitorList::MonitorList(VMThreadList* thread_list)
  : monitors_lock_("MonitorList lock"),
    zone_(thread_list->worker_runtime()->main_isolate()->allocator(), "monitor list zone"),
    monitors_(&zone_),
    thread_list_(thread_list) {
}


MonitorList::~MonitorList() {
  VMThread* self = VMThread::Current();
  MutexLock mu(self, monitors_lock_);
  MonitorPool::ReleaseMonitors(thread_list_, self, &monitors_);
}


void MonitorList::FreeMonitors() {
  VMThread* self = VMThread::Current();
  DCHECK(self->IsInSuspendAll());
  MonitorList* list = self->thread_list()->monitor_list();
  MutexLock mu(self, list->monitors_lock_);

  for (auto it = list->monitors_.begin(); it != list->monitors_.end();) {
    Monitor* m = *it;
    // JSMT: TODO should lock ?
    // m->monitor_lock_.Lock(self);
    if (!m->is_marked_) {
      // m->monitor_lock_.Unlock(self);
      MonitorPool::ReleaseMonitor(self, m);
      it = list->monitors_.erase(it);
    } else {
      m->is_marked_ = false;
      ++it;
      // m->monitor_lock_.Unlock(self);
    }
  }
}


void MonitorList::Add(Monitor* m) {
  VMThread* self = VMThread::Current();
  MutexLock mu(self, monitors_lock_);
  monitors_.push_front(m);
}


#ifdef __LP64__
MonitorPool::MonitorPool()
    : current_chunk_list_index_(0),
      num_chunks_(0),
      current_chunk_list_capacity_(0),
      allocated_monitor_ids_lock_("allocated monitor ids lock"),
      first_free_(nullptr) {
  for (size_t i = 0; i < kMaxChunkLists; ++i) {
    monitor_chunks_[i] = nullptr;  // Not absolutely required, but ...
  }
  AllocateChunk();  // Get our first chunk.
}
#endif

Monitor* MonitorPool::CreateMonitor(VMThread* self, VMThread* owner) {
#ifndef __LP64__
  Monitor* mon = new Monitor(self, owner);
  DCHECK((reinterpret_cast<intptr_t>(mon) & LockWord::kMonitorIdAlignmentMask) == 0);
  return mon;
#else
  return monitor_pool(self)->CreateMonitorInPool(self, owner);
#endif
}


void MonitorPool::ReleaseMonitor(VMThread* self, Monitor* monitor) {
#ifndef __LP64__
  USE(self);
  delete monitor;
#else
  monitor_pool(self)->ReleaseMonitorToPool(self, monitor);
#endif
}


void MonitorPool::ReleaseMonitors(VMThreadList* thread_list,
                                  VMThread* self,
                                  MonitorList::Monitors* monitors) {
  for (const auto& mon : *monitors) {
#ifndef __LP64__
    USE(thread_list);
    USE(self);
    delete mon;
#else
    thread_list->monitor_pool()->ReleaseMonitorToPool(self, mon);
#endif
  }
}


Monitor* MonitorPool::MonitorFromMonitorId(VMThread* self, MonitorId mon_id) {
#ifndef __LP64__
  return reinterpret_cast<Monitor*>(
      mon_id << LockWord::kMonitorIdAlignmentShift);
#else
  return monitor_pool(self)->LookupMonitor(mon_id);
#endif
}


MonitorId MonitorPool::MonitorIdFromMonitor(Monitor* mon) {
#ifndef __LP64__
  return reinterpret_cast<MonitorId>(mon) >> LockWord::kMonitorIdAlignmentShift;
#else
  return mon->GetMonitorId();
#endif
}


MonitorId MonitorPool::ComputeMonitorId(Monitor* mon, VMThread* self) {
#ifndef __LP64__
  USE(self);
  return MonitorIdFromMonitor(mon);
#else
  return monitor_pool(self)->ComputeMonitorIdInPool(mon, self);
#endif
}


MonitorPool* MonitorPool::monitor_pool(VMThread* self) {
#ifndef __LP64__
  return NULL;
#else
  return self->thread_list()->monitor_pool();
#endif
}

#ifdef __LP64__
// Assumes locks are held appropriately when necessary.
// We do not need a lock in the constructor, but we need one when in CreateMonitorInPool.
void MonitorPool::AllocateChunk() {
  DCHECK(first_free_ == nullptr);

  // Do we need to allocate another chunk list?
  if (num_chunks_ == current_chunk_list_capacity_) {
    if (current_chunk_list_capacity_ != 0U) {
      ++current_chunk_list_index_;
      if (current_chunk_list_index_ >= kMaxChunkLists) {
        V8_Fatal(__FILE__, __LINE__, "Out of space for inflated monitors.");
      }
    }  // else we're initializing
    current_chunk_list_capacity_ = ChunkListCapacity(current_chunk_list_index_);
    uintptr_t* new_list = new uintptr_t[current_chunk_list_capacity_]();
    DCHECK(monitor_chunks_[current_chunk_list_index_] == nullptr);
    monitor_chunks_[current_chunk_list_index_] = new_list;
    num_chunks_ = 0;
  }

  // Allocate the chunk.
  void* chunk = Malloced::New(kChunkSize);
  // Check we allocated memory.
  CHECK_NE(reinterpret_cast<uintptr_t>(nullptr), reinterpret_cast<uintptr_t>(chunk));
  // Check it is aligned as we need it.
  CHECK_EQ(0U, reinterpret_cast<uintptr_t>(chunk) % kMonitorAlignment);

  // Add the chunk.
  monitor_chunks_[current_chunk_list_index_][num_chunks_] = reinterpret_cast<uintptr_t>(chunk);
  num_chunks_++;

  // Set up the free list
  Monitor* last = reinterpret_cast<Monitor*>(reinterpret_cast<uintptr_t>(chunk) +
                                             (kChunkCapacity - 1) * kAlignedMonitorSize);
  last->next_free_ = nullptr;
  // Eagerly compute id.
  last->monitor_id_ = OffsetToMonitorId(current_chunk_list_index_* (kMaxListSize * kChunkSize)
      + (num_chunks_ - 1) * kChunkSize + (kChunkCapacity - 1) * kAlignedMonitorSize);
  for (size_t i = 0; i < kChunkCapacity - 1; ++i) {
    Monitor* before = reinterpret_cast<Monitor*>(reinterpret_cast<uintptr_t>(last) -
                                                 kAlignedMonitorSize);
    before->next_free_ = last;
    // Derive monitor_id from last.
    before->monitor_id_ = OffsetToMonitorId(MonitorIdToOffset(last->monitor_id_) -
                                            kAlignedMonitorSize);

    last = before;
  }
  DCHECK(last == reinterpret_cast<Monitor*>(chunk));
  first_free_ = last;
}

void MonitorPool::FreeInternal() {
  // This is on shutdown with NO_THREAD_SAFETY_ANALYSIS, can't/don't need to lock.
  DCHECK_NE(current_chunk_list_capacity_, 0UL);
  for (size_t i = 0; i <= current_chunk_list_index_; ++i) {
    DCHECK_NE(monitor_chunks_[i], static_cast<uintptr_t*>(nullptr));
    for (size_t j = 0; j < ChunkListCapacity(i); ++j) {
      if (i < current_chunk_list_index_ || j < num_chunks_) {
        DCHECK_NE(monitor_chunks_[i][j], 0U);
        Malloced::Delete(reinterpret_cast<void*>(monitor_chunks_[i][j]));
      } else {
        DCHECK_EQ(monitor_chunks_[i][j], 0U);
      }
    }
    delete[] monitor_chunks_[i];
  }
}

Monitor* MonitorPool::CreateMonitorInPool(VMThread* self, VMThread* owner) {
  // We are gonna allocate, so acquire the writer lock.
  MutexLock mu(self, allocated_monitor_ids_lock_);

  // Enough space, or need to resize?
  if (first_free_ == nullptr) {
    AllocateChunk();
  }

  Monitor* mon_uninitialized = first_free_;
  first_free_ = first_free_->next_free_;

  // Pull out the id which was preinitialized.
  MonitorId id = mon_uninitialized->monitor_id_;

  // Initialize it.
  Monitor* monitor = new(mon_uninitialized) Monitor(self, owner, id);

  return monitor;
}

void MonitorPool::ReleaseMonitorToPool(VMThread* self, Monitor* monitor) {
  // Might be racy with allocation, so acquire lock.
  MutexLock mu(self, allocated_monitor_ids_lock_);

  // Keep the monitor id. Don't trust it's not cleared.
  MonitorId id = monitor->monitor_id_;

  // Call the destructor.
  // TODO: Exception safety?
  monitor->~Monitor();

  // Add to the head of the free list.
  monitor->next_free_ = first_free_;
  first_free_ = monitor;

  // Rewrite monitor id.
  monitor->monitor_id_ = id;
}
#endif

}  // namespace sync
}  // namespace internal
}  // namespace v8
