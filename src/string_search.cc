#include "string_search.h"

namespace node {
namespace stringsearch {

thread_local int StringSearchBase::kBadCharShiftTable[kUC16AlphabetSize];
thread_local int StringSearchBase::kGoodSuffixShiftTable[kBMMaxShift + 1];
thread_local int StringSearchBase::kSuffixTable[kBMMaxShift + 1];

}  // namespace stringsearch
}  // namespace node
