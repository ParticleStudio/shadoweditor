module ipc.shared_memory;

import <cstdint>;

//#include "Poco/Exception.h"
//#if defined(POCO_NO_SHAREDMEMORY)
//#    include "SharedMemory_DUMMY.cpp"
//#elif defined(POCO_OS_FAMILY_WINDOWS)
//#    include "SharedMemory_WIN32.cpp"
//#elif defined(POCO_OS_FAMILY_UNIX)
//#    include "SharedMemory_POSIX.cpp"
//#else
//#    include "SharedMemory_DUMMY.cpp"
//#endif


namespace ipc {
SharedMemory::SharedMemory() {
}

SharedMemory::SharedMemory(const std::string &name, std::size_t size, SharedMemoryAccessMode mode, const void *addrHint, bool server): m_pSharedMemoryImpl(new SharedMemoryImpl(name, size, mode, addrHint, server)) {
}

SharedMemory::SharedMemory(const File &rFile, SharedMemoryAccessMode mode, const void *pAddrHint): m_pSharedMemoryImpl(new SharedMemoryImpl(rFile, mode, pAddrHint)) {
}

SharedMemory::SharedMemory(const SharedMemory &other): m_pSharedMemoryImpl(other.m_pSharedMemoryImpl) {
    if(m_pSharedMemoryImpl)
        m_pSharedMemoryImpl->duplicate();
}

SharedMemory::~SharedMemory() {
    if(m_pSharedMemoryImpl)
        m_pSharedMemoryImpl->release();
}

SharedMemory &SharedMemory::operator=(const SharedMemory &other) {
    SharedMemory tmp(other);
    swap(tmp);
    return *this;
}

char *SharedMemory::begin() const {
    if(m_pSharedMemoryImpl)
        return m_pSharedMemoryImpl->begin();
    else
        return 0;
}

char *SharedMemory::end() const {
    if(m_pSharedMemoryImpl)
        return m_pSharedMemoryImpl->end();
    else
        return 0;
}

}// namespace ipc

// module ipc.shared_memory;
