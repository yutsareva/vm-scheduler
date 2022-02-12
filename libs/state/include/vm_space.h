#pragma once

#include <string>


namespace vm_scheduler {

using VmSpaceId = std::string;

// TODO: use or remove?
class VmSpace {
public:
    explicit VmSpace(VmSpaceId id) : id_(std::move(id)){};
    VmSpaceId id() { return id_; };

private:
    VmSpaceId id_;
};

} // namespace vm_scheduler
