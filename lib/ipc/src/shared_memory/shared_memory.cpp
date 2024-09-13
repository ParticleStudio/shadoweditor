module;

module ipc.shared_memory;

import <cstdint>;
import <cstring>;

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
namespace shared_memory {
SharedMemory::SharedMemory() {
}

SharedMemory::SharedMemory(const std::string &name, std::int64_t size, AccessMode mode, const void *addrHint, bool server) {
}

SharedMemory::SharedMemory(const Poco::File &file, AccessMode mode, const void *addrHint) {
}

SharedMemory::SharedMemory(const SharedMemory &other) {
    //    if(_pImpl)
    //        _pImpl->duplicate();
}

SharedMemory::~SharedMemory() {
}

SharedMemory &SharedMemory::operator=(const SharedMemory &other) {
    SharedMemory tmp(other);
    swap(tmp);
    return *this;
}

char *SharedMemory::begin() const {
    if(_pImpl)
        return _pImpl->begin();
    else
        return 0;
}

char *SharedMemory::end() const {
    if(_pImpl)
        return _pImpl->end();
    else
        return 0;
}
}// namespace shared_memory
}// namespace ipc

// module SharedMemory;
// module;