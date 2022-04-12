#include "libs/common/include/slot.h"

#include <iostream>

namespace vm_scheduler {

std::ostream& operator<<(std::ostream& out, const SlotCapacity& capacity)
{
    out << "(cpu: " << capacity.cpu.count() << " cores, "
        << "ram: " << capacity.ram.count() << " MB)";
    return out;
}

} // namespace vm_scheduler
