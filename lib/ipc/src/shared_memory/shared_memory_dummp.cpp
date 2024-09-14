module ipc.shared_memory.dummp;

#if defined(NO_SHAREDMEMORY)

import ipc.shared_memory.common;

namespace ipc {
SharedMemoryImpl::SharedMemoryImpl(const std::string &, std::size_t, SharedMemoryAccessMode, const void *, bool) {
}


SharedMemoryImpl::SharedMemoryImpl(const File &, SharedMemoryAccessMode, const void *) {
}


SharedMemoryImpl::~SharedMemoryImpl() {
}
}// namespace ipc

// module ipc.shared_memory.data;

#endif// #if defined(NO_SHAREDMEMORY)

// module ipc.shared_memory.dummp;
