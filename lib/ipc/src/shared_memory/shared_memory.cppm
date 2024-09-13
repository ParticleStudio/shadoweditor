module;

#include "ipc/ipc_common.h"

export module ipc.shared_memory;

import <algorithm>;
import <cstddef>;
import <string>;


namespace ipc {
export class File;

namespace shared_memory {
export class IPC_API SharedMemory {
    /// Create and manage a shared memory object.
    ///
    /// A SharedMemory object has value semantics, but
    /// is implemented using a handle/implementation idiom.
    /// Therefore, multiple SharedMemory objects can share
    /// a single, reference counted SharedMemoryImpl object.

 public:
    enum AccessMode {
        AM_READ = 0,
        AM_WRITE
    };

    SharedMemory();
    /// Default constructor creates an unmapped SharedMemory object.
    /// No clients can connect to an unmapped SharedMemory object.

    SharedMemory(const std::string &name, std::size_t size, AccessMode mode, const void *addrHint = 0, bool server = true);
    /// Creates or connects to a shared memory object with the given name.
    ///
    /// For maximum portability, name should be a valid Unix filename and not
    /// contain any slashes or backslashes.
    ///
    /// An address hint can be passed to the system, specifying the desired
    /// start address of the shared memory area. Whether the hint
    /// is actually honored is, however, up to the system. Windows platform
    /// will generally ignore the hint.
    ///
    /// If server is set to true, the shared memory region will be unlinked
    /// by calling shm_unlink() (on POSIX platforms) when the SharedMemory object is destroyed.
    /// The server parameter is ignored on Windows platforms.

    SharedMemory(const File &file, AccessMode mode, const void *addrHint = 0);
    /// Maps the entire contents of file into a shared memory segment.
    ///
    /// An address hint can be passed to the system, specifying the desired
    /// start address of the shared memory area. Whether the hint
    /// is actually honored is, however, up to the system. Windows platform
    /// will generally ignore the hint.

    SharedMemory(const SharedMemory &other);
    /// Creates a SharedMemory object by copying another one.

    ~SharedMemory();
    /// Destroys the SharedMemory.

    SharedMemory &operator=(const SharedMemory &other);
    /// Assigns another SharedMemory object.

    void swap(SharedMemory &other) noexcept;
    /// Swaps the SharedMemory object with another one.

    char *begin() const;
    /// Returns the start address of the shared memory segment.
    /// Will be NULL for illegal segments.

    char *end() const;
    /// Returns the one-past-end end address of the shared memory segment.
    /// Will be NULL for illegal segments.

 private:
};

//
// inlines
//
inline void SharedMemory::swap(SharedMemory &other) noexcept {
    //    std::swap(_pImpl, other._pImpl);
}
}// namespace shared_memory
}// namespace ipc

// module SharedMemory;
// module;
