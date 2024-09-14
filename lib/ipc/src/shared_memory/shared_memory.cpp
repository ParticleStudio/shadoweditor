module;

module ipc.shared_memory;

import <cstdint>;
import <string>;

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
SharedMemory::SharedMemory(): pSharedMemoryData(0) {
}

SharedMemory::SharedMemory(const std::string &name, std::size_t size, AccessMode mode, const void *addrHint, bool server): pSharedMemoryData(new SharedMemoryImpl(name, size, mode, addrHint, server)) {
}

SharedMemory::SharedMemory(const Poco::File &file, AccessMode mode, const void *addrHint): pSharedMemoryData(new SharedMemoryImpl(file, mode, addrHint)) {
}

SharedMemory::SharedMemory(const SharedMemory &other): pSharedMemoryData(other.pSharedMemoryData) {
    if(pSharedMemoryData)
        pSharedMemoryData->duplicate();
}

SharedMemory::~SharedMemory() {
    if(pSharedMemoryData)
        pSharedMemoryData->release();
}

SharedMemory &SharedMemory::operator=(const SharedMemory &other) {
    SharedMemory tmp(other);
    swap(tmp);
    return *this;
}

char *SharedMemory::begin() const {
    if(pSharedMemoryData)
        return pSharedMemoryData->begin();
    else
        return 0;
}

char *SharedMemory::end() const {
    if(pSharedMemoryData)
        return pSharedMemoryData->end();
    else
        return 0;
}

}// namespace ipc

// module SharedMemory;
// module;