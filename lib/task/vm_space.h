#pragma once

#include <string>

namespace vm_scheduler::task {

using VmSpaceId = std::string;

class VmSpace {
public:
    explicit VmSpace(VmSpaceId id) : id_(std::move(id)){};
    VmSpaceId id() { return id_; };

private:
    VmSpaceId id_;
};

} // namespace vm_scheduler::task
