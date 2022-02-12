#pragma once

#include "task_blob.h"


// TODO: remove, use dockerhub instead
namespace vm_scheduler::blob_storage {
class BlobStorage {
    virtual void add(const TaskBinary& taskBinary) = 0;
};
} // namespace vm_scheduler::blob_storage
