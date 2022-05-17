#pragma once

#include <libs/common/include/slot.h>
#include <libs/state/include/state.h>

#include <vector>

namespace vm_scheduler::testing {

std::vector<SlotCapacity> getPossibleSlots();
std::vector<SlotCapacity> getC6gPossibleSlots();

void checkStateConstrains(const State& state, const StateChange& stateChange);

} // namespace vm_scheduler::testing
