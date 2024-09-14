module;

#include "ipc/ipc_common.h"

#if defined(NO_SHAREDMEMORY)

export module ipc.shared_memory.data;

import <string>;

import ipc.shared_memory;

namespace ipc {
class IPC_API SharedMemoryData {
    /// A dummy implementation of shared memory, for systems
    /// that do not have shared memory support.

 public:
    SharedMemoryData(const std::string &id, std::size_t size, SharedMemory::AccessMode mode, const void *addr, bool server);
    /// Creates or connects to a shared memory object with the given name.
    ///
    /// For maximum portability, name should be a valid Unix filename and not
    /// contain any slashes or backslashes.
    ///
    /// An address hint can be passed to the system, specifying the desired
    /// start address of the shared memory area. Whether the hint
    /// is actually honored is, however, up to the system. Windows platform
    /// will generally ignore the hint.

    SharedMemoryData(const File &aFile, SharedMemory::AccessMode mode, const void *addr);
    /// Maps the entire contents of file into a shared memory segment.
    ///
    /// An address hint can be passed to the system, specifying the desired
    /// start address of the shared memory area. Whether the hint
    /// is actually honored is, however, up to the system. Windows platform
    /// will generally ignore the hint.

    char *begin() const;
    /// Returns the start address of the shared memory segment.

    char *end() const;
    /// Returns the one-past-end end address of the shared memory segment.

 protected:
    ~SharedMemoryData();
    /// Destroys the SharedMemoryData.

 private:
    SharedMemoryData();
    SharedMemoryData(const SharedMemoryData &);
    SharedMemoryData &operator=(const SharedMemoryData &);
};


//
// inlines
//
inline char *SharedMemoryData::begin() const {
    return 0;
}


inline char *SharedMemoryData::end() const {
    return 0;
}
}// namespace ipc

// module ipc.shared_memory.data;

#endif// #if defined(NO_SHAREDMEMORY)

// module;
