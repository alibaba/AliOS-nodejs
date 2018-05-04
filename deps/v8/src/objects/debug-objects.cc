// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/objects/debug-objects.h"
#include "src/objects/debug-objects-inl.h"

namespace v8 {
namespace internal {

bool DebugInfo::IsEmpty() const { return flags() == kNone; }

bool DebugInfo::HasBreakInfo() const { return (flags() & kHasBreakInfo) != 0; }

bool DebugInfo::ClearBreakInfo() {
  Isolate* isolate = GetIsolate();

  set_debug_bytecode_array(isolate->heap()->undefined_value());
  RemoveBreakPoints();

  int new_flags = flags() & ~kHasBreakInfo;
  set_flags(new_flags);

  return new_flags == kNone;
}

// Check if there is a break point at this source position.
bool DebugInfo::HasBreakPoint(int source_position) {
  DCHECK(HasBreakInfo());
  // Get the break point info object for this code offset.
  Object* break_point_info = GetBreakPointInfo(source_position);

  // If there is no break point info object or no break points in the break
  // point info object there is no break point at this code offset.
  if (break_point_info->IsUndefined(GetIsolate())) return false;
  return BreakPointInfo::cast(break_point_info)->GetBreakPointCount() > 0;
}

// Get the break point info object for this source position.
Object* DebugInfo::GetBreakPointInfo(int source_position) {
  DCHECK(HasBreakInfo());
  Isolate* isolate = GetIsolate();
  FixedArray* break_points = BreakPoints();
  if (!break_points->IsUndefined(isolate)) {
    for (int i = 0; i < break_points->length(); i++) {
      if (!break_points->get(i)->IsUndefined(isolate)) {
        BreakPointInfo* break_point_info =
            BreakPointInfo::cast(break_points->get(i));
        if (break_point_info->source_position() == source_position) {
          return break_point_info;
        }
      }
    }
  }
  return isolate->heap()->undefined_value();
}

FixedArray* DebugInfo::BreakPoints() {
  Isolate* isolate = GetIsolate();
  HandleScope scope(isolate);
  if (isolate->heap()->weak_debug_points_table() == Smi::kZero) {
    return reinterpret_cast<FixedArray*>(isolate->heap()->undefined_value());
  }

  Handle<WeakHashTable> table(
      WeakHashTable::cast(isolate->heap()->weak_debug_points_table()), isolate);
  Handle<DebugInfo> debug_info(this, isolate);
  Object* result = table->Lookup(debug_info);
  if (isolate->heap()->the_hole_value() == result) {
    return reinterpret_cast<FixedArray*>(isolate->heap()->undefined_value());
  }

  return FixedArray::cast(result);
}

void DebugInfo::SetBreakPoints(FixedArray* value) {
  Isolate* isolate = GetIsolate();
  HandleScope scope(isolate);
  Handle<FixedArray> points(value, isolate);
  CHECK(!points->is_shared_object());
  if (isolate->heap()->weak_debug_points_table() == Smi::kZero) {
    isolate->heap()->SetRootWeakDebugPointsTable(
        *WeakHashTable::New(isolate, 16, TENURED));
  }
  Heap* heap = isolate->heap();
  Handle<WeakHashTable> table(
      WeakHashTable::cast(heap->weak_debug_points_table()), isolate);
  Handle<DebugInfo> debug_info(this, isolate);
  table = WeakHashTable::Put(table, debug_info, points);
  heap->SetRootWeakDebugPointsTable(*table);
  sync::ThreadFenceForConstructor();
  DCHECK_EQ(*points, BreakPoints());
}

void DebugInfo::RemoveBreakPoints() {
  Isolate* isolate = GetIsolate();
  HandleScope scope(isolate);
  Heap* heap = isolate->heap();
  Handle<WeakHashTable> table(
      WeakHashTable::cast(heap->weak_debug_points_table()), isolate);
  Handle<DebugInfo> debug_info(this, isolate);
  WeakHashTable::Remove(table, debug_info);
  sync::ThreadFenceForConstructor();
}

bool DebugInfo::ClearBreakPoint(Handle<DebugInfo> debug_info,
                                Handle<Object> break_point_object) {
  DCHECK(debug_info->HasBreakInfo());
  Isolate* isolate = debug_info->GetIsolate();
  FixedArray* break_points = debug_info->BreakPoints();
  if (break_points->IsUndefined(isolate)) return false;

  for (int i = 0; i < break_points->length(); i++) {
    if (break_points->get(i)->IsUndefined(isolate)) continue;
    Handle<BreakPointInfo> break_point_info = Handle<BreakPointInfo>(
        BreakPointInfo::cast(break_points->get(i)), isolate);
    if (BreakPointInfo::HasBreakPointObject(break_point_info,
                                            break_point_object)) {
      BreakPointInfo::ClearBreakPoint(break_point_info, break_point_object);
      return true;
    }
  }
  return false;
}

void DebugInfo::SetBreakPoint(Handle<DebugInfo> debug_info, int source_position,
                              Handle<Object> break_point_object) {
  DCHECK(debug_info->HasBreakInfo());
  Isolate* isolate = debug_info->GetIsolate();
  Handle<Object> break_point_info(
      debug_info->GetBreakPointInfo(source_position), isolate);
  if (!break_point_info->IsUndefined(isolate)) {
    BreakPointInfo::SetBreakPoint(
        Handle<BreakPointInfo>::cast(break_point_info), break_point_object);
    return;
  }

  // Adding a new break point for a code offset which did not have any
  // break points before. Try to find a free slot.
  static const int kNoBreakPointInfo = -1;
  int index = kNoBreakPointInfo;
  HandleScope scope(isolate);
  Handle<FixedArray> break_points = Handle<FixedArray>(debug_info->BreakPoints(), isolate);
  // if no break points.
  if (break_points->IsUndefined(isolate)) {
    break_points = isolate->factory()->NewFixedArray(
            DebugInfo::kEstimatedNofBreakPointsInFunction, TENURED);
    debug_info->SetBreakPoints(*break_points);
    break_points = Handle<FixedArray>(debug_info->BreakPoints(), isolate);
  }

  for (int i = 0; i < break_points->length(); i++) {
    if (break_points->get(i)->IsUndefined(isolate)) {
      index = i;
      break;
    }
  }

  if (index == kNoBreakPointInfo) {
    Handle<FixedArray> new_break_points = isolate->factory()->NewFixedArray(
            break_points->length() +
            DebugInfo::kEstimatedNofBreakPointsInFunction, TENURED);
    debug_info->SetBreakPoints(*new_break_points);
    for (int i = 0; i < break_points->length(); i++) {
      new_break_points->set(i, break_points->get(i));
    }
    index = break_points->length();
  }
  DCHECK(index != kNoBreakPointInfo);

  // Allocate new BreakPointInfo object and set the break point.
  Handle<BreakPointInfo> new_break_point_info =
      isolate->factory()->NewBreakPointInfo(source_position, TENURED);
  BreakPointInfo::SetBreakPoint(new_break_point_info, break_point_object);
  debug_info->BreakPoints()->set(index, *new_break_point_info);
}

// Get the break point objects for a source position.
Handle<Object> DebugInfo::GetBreakPointObjects(int source_position) {
  DCHECK(HasBreakInfo());
  Object* break_point_info = GetBreakPointInfo(source_position);
  Isolate* isolate = GetIsolate();
  if (break_point_info->IsUndefined(isolate)) {
    return isolate->factory()->undefined_value();
  }
  return Handle<Object>(
      BreakPointInfo::cast(break_point_info)->break_point_objects(), isolate);
}

// Get the total number of break points.
int DebugInfo::GetBreakPointCount() {
  DCHECK(HasBreakInfo());
  Isolate* isolate = GetIsolate();
  FixedArray* break_points = BreakPoints();
  if (break_points->IsUndefined(isolate)) return 0;
  int count = 0;
  for (int i = 0; i < break_points->length(); i++) {
    if (!break_points->get(i)->IsUndefined(isolate)) {
      BreakPointInfo* break_point_info =
          BreakPointInfo::cast(break_points->get(i));
      count += break_point_info->GetBreakPointCount();
    }
  }
  return count;
}

Handle<Object> DebugInfo::FindBreakPointInfo(
    Handle<DebugInfo> debug_info, Handle<Object> break_point_object) {
  DCHECK(debug_info->HasBreakInfo());
  Isolate* isolate = debug_info->GetIsolate();
  FixedArray* break_points = debug_info->BreakPoints();
  if (!break_points->IsUndefined(isolate)) {
    for (int i = 0; i < break_points->length(); i++) {
      if (!break_points->get(i)->IsUndefined(isolate)) {
        Handle<BreakPointInfo> break_point_info = Handle<BreakPointInfo>(
            BreakPointInfo::cast(break_points->get(i)), isolate);
        if (BreakPointInfo::HasBreakPointObject(break_point_info,
                                                break_point_object)) {
          return break_point_info;
        }
      }
    }
  }
  return isolate->factory()->undefined_value();
}

bool DebugInfo::HasCoverageInfo() const {
  return (flags() & kHasCoverageInfo) != 0;
}

bool DebugInfo::ClearCoverageInfo() {
  DCHECK(FLAG_block_coverage);
  DCHECK(HasCoverageInfo());
  Isolate* isolate = GetIsolate();

  set_coverage_info(isolate->heap()->undefined_value());

  int new_flags = flags() & ~kHasCoverageInfo;
  set_flags(new_flags);

  return new_flags == kNone;
}

// Remove the specified break point object.
void BreakPointInfo::ClearBreakPoint(Handle<BreakPointInfo> break_point_info,
                                     Handle<Object> break_point_object) {
  Isolate* isolate = break_point_info->GetIsolate();
  // If there are no break points just ignore.
  if (break_point_info->break_point_objects()->IsUndefined(isolate)) return;
  // If there is a single break point clear it if it is the same.
  if (!break_point_info->break_point_objects()->IsFixedArray()) {
    if (break_point_info->break_point_objects() == *break_point_object) {
      break_point_info->set_break_point_objects(
          isolate->heap()->undefined_value());
    }
    return;
  }
  // If there are multiple break points shrink the array
  DCHECK(break_point_info->break_point_objects()->IsFixedArray());
  Handle<FixedArray> old_array = Handle<FixedArray>(
      FixedArray::cast(break_point_info->break_point_objects()));
  Handle<FixedArray> new_array =
      isolate->factory()->NewFixedArray(old_array->length() - 1, TENURED);
  int found_count = 0;
  for (int i = 0; i < old_array->length(); i++) {
    if (old_array->get(i) == *break_point_object) {
      DCHECK(found_count == 0);
      found_count++;
    } else {
      new_array->set(i - found_count, old_array->get(i));
    }
  }
  // If the break point was found in the list change it.
  if (found_count > 0) break_point_info->set_break_point_objects(*new_array);
}

// Add the specified break point object.
void BreakPointInfo::SetBreakPoint(Handle<BreakPointInfo> break_point_info,
                                   Handle<Object> break_point_object) {
  Isolate* isolate = break_point_info->GetIsolate();

  // If there was no break point objects before just set it.
  if (break_point_info->break_point_objects()->IsUndefined(isolate)) {
    break_point_info->set_break_point_objects(*break_point_object);
    return;
  }

  // If the break point object is the same as before just ignore.
  if (break_point_info->break_point_objects() == *break_point_object) return;

  // If there was one break point object before replace with array.
  if (!break_point_info->break_point_objects()->IsFixedArray()) {
    Handle<FixedArray> array = isolate->factory()->NewFixedArray(2, TENURED);
    array->set(0, break_point_info->break_point_objects());
    array->set(1, *break_point_object);
    break_point_info->set_break_point_objects(*array);
    return;
  }
  // If there was more than one break point before extend array.
  Handle<FixedArray> old_array = Handle<FixedArray>(
      FixedArray::cast(break_point_info->break_point_objects()));
  Handle<FixedArray> new_array =
      isolate->factory()->NewFixedArray(old_array->length() + 1, TENURED);
  for (int i = 0; i < old_array->length(); i++) {
    // If the break point was there before just ignore.
    if (old_array->get(i) == *break_point_object) return;
    new_array->set(i, old_array->get(i));
  }
  // Add the new break point.
  new_array->set(old_array->length(), *break_point_object);
  break_point_info->set_break_point_objects(*new_array);
}

bool BreakPointInfo::HasBreakPointObject(
    Handle<BreakPointInfo> break_point_info,
    Handle<Object> break_point_object) {
  // No break point.
  Isolate* isolate = break_point_info->GetIsolate();
  if (break_point_info->break_point_objects()->IsUndefined(isolate)) {
    return false;
  }
  // Single break point.
  if (!break_point_info->break_point_objects()->IsFixedArray()) {
    return break_point_info->break_point_objects() == *break_point_object;
  }
  // Multiple break points.
  FixedArray* array = FixedArray::cast(break_point_info->break_point_objects());
  for (int i = 0; i < array->length(); i++) {
    if (array->get(i) == *break_point_object) {
      return true;
    }
  }
  return false;
}

// Get the number of break points.
int BreakPointInfo::GetBreakPointCount() {
  // No break point.
  if (break_point_objects()->IsUndefined(GetIsolate())) return 0;
  // Single break point.
  if (!break_point_objects()->IsFixedArray()) return 1;
  // Multiple break points.
  return FixedArray::cast(break_point_objects())->length();
}

int CoverageInfo::SlotCount() const {
  DCHECK(FLAG_block_coverage);
  DCHECK_EQ(kFirstSlotIndex, length() % kSlotIndexCount);
  return (length() - kFirstSlotIndex) / kSlotIndexCount;
}

int CoverageInfo::StartSourcePosition(int slot_index) const {
  DCHECK(FLAG_block_coverage);
  DCHECK_LT(slot_index, SlotCount());
  const int slot_start = CoverageInfo::FirstIndexForSlot(slot_index);
  return Smi::ToInt(get(slot_start + kSlotStartSourcePositionIndex));
}

int CoverageInfo::EndSourcePosition(int slot_index) const {
  DCHECK(FLAG_block_coverage);
  DCHECK_LT(slot_index, SlotCount());
  const int slot_start = CoverageInfo::FirstIndexForSlot(slot_index);
  return Smi::ToInt(get(slot_start + kSlotEndSourcePositionIndex));
}

int CoverageInfo::BlockCount(int slot_index) const {
  DCHECK(FLAG_block_coverage);
  DCHECK_LT(slot_index, SlotCount());
  const int slot_start = CoverageInfo::FirstIndexForSlot(slot_index);
  return Smi::ToInt(get(slot_start + kSlotBlockCountIndex));
}

void CoverageInfo::InitializeSlot(int slot_index, int from_pos, int to_pos) {
  DCHECK(FLAG_block_coverage);
  DCHECK_LT(slot_index, SlotCount());
  const int slot_start = CoverageInfo::FirstIndexForSlot(slot_index);
  set(slot_start + kSlotStartSourcePositionIndex, Smi::FromInt(from_pos));
  set(slot_start + kSlotEndSourcePositionIndex, Smi::FromInt(to_pos));
  set(slot_start + kSlotBlockCountIndex, Smi::kZero);
}

void CoverageInfo::IncrementBlockCount(int slot_index) {
  DCHECK(FLAG_block_coverage);
  DCHECK_LT(slot_index, SlotCount());
  const int slot_start = CoverageInfo::FirstIndexForSlot(slot_index);
  const int old_count = BlockCount(slot_index);
  set(slot_start + kSlotBlockCountIndex, Smi::FromInt(old_count + 1));
}

void CoverageInfo::ResetBlockCount(int slot_index) {
  DCHECK(FLAG_block_coverage);
  DCHECK_LT(slot_index, SlotCount());
  const int slot_start = CoverageInfo::FirstIndexForSlot(slot_index);
  set(slot_start + kSlotBlockCountIndex, Smi::kZero);
}

}  // namespace internal
}  // namespace v8
