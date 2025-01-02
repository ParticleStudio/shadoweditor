module;

#include "Windows.h"
#include "ipc/common.h"

export module ipc.shared_memory.windows;

import shadow.platform;

#if defined(PLATFORM_OS_FAMILY_WINDOWS)

import <string>;

import ipc.shared_memory.common;

namespace ipc {
export class File;

export class IPC_API SharedMemoryImpl {
    /// Shared memory implementation for Windows platforms.

 public:
    SharedMemoryImpl(const std::string &, std::size_t, SharedMemoryAccessMode, const void *, bool);
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

    ~SharedMemoryImpl();
    /// Destroys the SharedMemoryImpl.

 private:
    SharedMemoryImpl();
    SharedMemoryImpl(const SharedMemoryImpl &);
    SharedMemoryImpl &operator=(const SharedMemoryImpl &);

    std::string m_name;
    HANDLE m_memoryHandle;
    HANDLE m_fileHandle;
    std::size_t m_size;
    DWORD m_mode;
    char *m_pAddress;
};


//
// inlines
//
inline char *SharedMemoryImpl::begin() const {
    return m_pAddress;
}


inline char *SharedMemoryImpl::end() const {
    return m_pAddress + m_size;
}
}// namespace ipc

#endif// #if defined(PLATFORM_OS_FAMILY_WINDOWS)

// module ipc.shared_memory.windows;
// module;
