module;

export module ipc.shared_memory.dummp;

import shadow.platform;

#if defined(NO_SHAREDMEMORY)

import <string>;

import ipc.shared_memory;

#include "ipc/ipc_common.h"

namespace ipc {
class IPC_API SharedMemoryImpl {
    /// A dummy implementation of shared memory, for systems
    /// that do not have shared memory support.

 public:
    SharedMemoryImpl(const std::string &, std::size_t size, SharedMemoryAccessMode, const void *, bool);
    /// Creates or connects to a shared memory object with the given name.
    ///
    /// For maximum portability, name should be a valid Unix filename and not
    /// contain any slashes or backslashes.
    ///
    /// An address hint can be passed to the system, specifying the desired
    /// start address of the shared memory area. Whether the hint
    /// is actually honored is, however, up to the system. Windows platform
    /// will generally ignore the hint.

    SharedMemoryImpl(const File &, SharedMemoryAccessMode, const void *);
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
    ~SharedMemoryImpl();
    /// Destroys the SharedMemoryImpl.

 private:
    SharedMemoryImpl();
    SharedMemoryImpl(const SharedMemoryImpl &);
    SharedMemoryImpl &operator=(const SharedMemoryImpl &);
};


//
// inlines
//
inline char *SharedMemoryImpl::begin() const {
    return 0;
}


inline char *SharedMemoryImpl::end() const {
    return 0;
}
}// namespace ipc

#endif// #if defined(NO_SHAREDMEMORY)

// module ipc.shared_memory.dummp;
// module;
