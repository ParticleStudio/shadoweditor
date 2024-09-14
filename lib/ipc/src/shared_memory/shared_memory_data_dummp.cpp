module;

#if defined(NO_SHAREDMEMORY)

module ipc.shared_memory.data;

import ipc.shared_memory;

namespace ipc {
SharedMemoryData::SharedMemoryData(const std::string&, std::size_t, SharedMemory::AccessMode, const void*, bool)
{
}


SharedMemoryData::SharedMemoryData(const Poco::File&, SharedMemory::AccessMode, const void*)
{
}


SharedMemoryData::~SharedMemoryData()
{
}
}// namespace ipc

// module ipc.shared_memory.data;

#endif// #if defined(NO_SHAREDMEMORY)

// module;
