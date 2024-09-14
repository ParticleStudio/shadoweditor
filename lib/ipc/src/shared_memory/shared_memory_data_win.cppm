module;

#include "minwindef.h"
#include "common/platform.hpp"
#include "ipc/ipc_common.h"

#if defined(PLATFORM_OS_FAMILY_WINDOWS)

export module ipc.shared_memory.data;

import <string>;

import ipc.shared_memory.common;

namespace ipc {
export class File;

export class IPC_API SharedMemoryData {
    /// Shared memory implementation for Windows platforms.

 public:
    SharedMemoryData(const std::string &name, std::size_t size, SharedMemoryAccessMode mode, const void *addrHint, bool server);
    /// Creates or connects to a shared memory object with the given name.
    ///
    /// For maximum portability, name should be a valid Unix filename and not
    /// contain any slashes or backslashes.
    ///
    /// An address hint can be passed to the system, specifying the desired
    /// start address of the shared memory area. Whether the hint
    /// is actually honored is, however, up to the system. Windows platform
    /// will generally ignore the hint.

    SharedMemoryData(const File &file, SharedMemoryAccessMode mode, const void *addrHint);
    /// Maps the entire contents of file into a shared memory segment.
    ///
    /// An address hint can be passed to the system, specifying the desired
    /// start address of the shared memory area. Whether the hint
    /// is actually honored is, however, up to the system. Windows platform
    /// will generally ignore the hint.

    char *begin() const;
    /// Returns the begin address of the SharedMemory segment. Will be null for illegal segments.

    char *end() const;
    /// Points past the last byte of the end address of the SharedMemory segment. Will be null for illegal segments.

 protected:
    void map();
    /// Maps the shared memory object.

    void unmap();
    /// Unmaps the shared memory object.

    void close();
    /// Releases the handle for the shared memory segment.

    ~SharedMemoryData();
    /// Destroys the SharedMemoryData.

 private:
    SharedMemoryData();
    SharedMemoryData(const SharedMemoryData &);
    SharedMemoryData &operator=(const SharedMemoryData &);

    std::string _name;
    HANDLE _memHandle;
    HANDLE _fileHandle;
    std::size_t _size;
    DWORD _mode;
    char *_address;
};


//
// inlines
//
inline char *SharedMemoryData::begin() const {
    return _address;
}


inline char *SharedMemoryData::end() const {
    return _address + _size;
}
}// namespace ipc

// module ipc.shared_memory;

#endif// #if defined(PLATFORM_OS_FAMILY_WINDOWS)

// module;
