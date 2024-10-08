module ipc.shared_memory.windows;

import <format>;
import <string>;

#include "minwindef.h"

namespace ipc {
SharedMemoryImpl::SharedMemoryImpl(const std::string &name, std::size_t size, SharedMemoryAccessMode mode, const void *, bool): m_name(name),
                                                                                                                                m_memoryHandle(INVALID_HANDLE_VALUE),
                                                                                                                                m_fileHandle(INVALID_HANDLE_VALUE),
                                                                                                                                m_size(size),
                                                                                                                                m_mode(PAGE_READONLY),
                                                                                                                                m_pAddress(0) {
    if(mode == SharedMemoryAccessMode::AM_WRITE)
        m_mode = PAGE_READWRITE;

    std::wstring utf16name;
    UnicodeConverter::toUTF16(m_name, utf16name);
#ifdef _WIN64
    const DWORD dwMaxSizeLow = static_cast<DWORD>(m_size & 0xFFFFFFFFULL);
    const DWORD dwMaxSizeHigh = static_cast<DWORD>((m_size & (0xFFFFFFFFULL << 32)) >> 32);
#else
    if(_size > std::numeric_limits<DWORD>::max()) {
        throw Poco::InvalidArgumentException(std::format("Requested shared memory size ({}) too large (max {})", _size, std::numeric_limits<DWORD>::max()));
    }
    const DWORD dwMaxSizeLow = static_cast<DWORD>(_size);
    const DWORD dwMaxSizeHigh = 0UL;
#endif
    m_memoryHandle = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, m_mode, dwMaxSizeHigh, dwMaxSizeLow, utf16name.c_str());

    if(!m_memoryHandle) {
        DWORD dwRetVal = GetLastError();
        int retVal = static_cast<int>(dwRetVal);

        if(m_mode != PAGE_READONLY or dwRetVal != 5) {
            throw SystemException(std::format("Cannot create shared memory object {} [Error {}: {}]", m_name, retVal, Error::getMessage(dwRetVal)), retVal);
        }

        m_memoryHandle = OpenFileMappingW(PAGE_READONLY, FALSE, utf16name.c_str());
        if(!m_memoryHandle) {
            dwRetVal = GetLastError();
            throw SystemException(std::format("Cannot open shared memory object {} [Error {}: {}]", m_name, retVal, Error::getMessage(dwRetVal)), retVal);
        }
    }
    map();
}


SharedMemoryImpl::SharedMemoryImpl(const File &file, SharedMemoryAccessMode mode, const void *): m_name(file.path()),
                                                                                                 m_memoryHandle(INVALID_HANDLE_VALUE),
                                                                                                 m_fileHandle(INVALID_HANDLE_VALUE),
                                                                                                 m_size(0),
                                                                                                 m_mode(PAGE_READONLY),
                                                                                                 m_pAddress(0) {
    if(!file.exists() or !file.isFile())
        throw FileNotFoundException(m_name);

    m_size = static_cast<DWORD>(file.getSize());

    DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD fileMode = GENERIC_READ;

    if(mode == SharedMemoryAccessMode::AM_WRITE) {
        m_mode = PAGE_READWRITE;
        fileMode |= GENERIC_WRITE;
    }

    std::wstring utf16name;
    UnicodeConverter::toUTF16(m_name, utf16name);
    m_fileHandle = CreateFileW(utf16name.c_str(), fileMode, shareMode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if(m_fileHandle == INVALID_HANDLE_VALUE)
        throw OpenFileException("Cannot open memory mapped file", m_name);

    m_memoryHandle = CreateFileMapping(m_fileHandle, NULL, m_mode, 0, 0, NULL);
    if(!m_memoryHandle) {
        DWORD dwRetVal = GetLastError();
        CloseHandle(m_fileHandle);
        m_fileHandle = INVALID_HANDLE_VALUE;
        throw SystemException(std::format("Cannot map file into shared memory {} [Error {}: {}]", m_name, (int)dwRetVal, Error::getMessage(dwRetVal)));
    }
    map();
}


SharedMemoryImpl::~SharedMemoryImpl() {
    unmap();
    close();
}


void SharedMemoryImpl::map() {
    DWORD access = FILE_MAP_READ;
    if(m_mode == PAGE_READWRITE)
        access = FILE_MAP_WRITE;
    LPVOID addr = MapViewOfFile(m_memoryHandle, access, 0, 0, m_size);
    if(!addr) {
        DWORD dwRetVal = GetLastError();
        throw SystemException(std::format("Cannot map shared memory object {} [Error {}: {}]", m_name, (int)dwRetVal, Error::getMessage(dwRetVal)));
    }

    m_pAddress = static_cast<char *>(addr);
}


void SharedMemoryImpl::unmap() {
    if(m_pAddress) {
        UnmapViewOfFile(m_pAddress);
        m_pAddress = 0;
        return;
    }
}


void SharedMemoryImpl::close() {
    if(_memHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_memoryHandle);
        _memHandle = INVALID_HANDLE_VALUE;
    }

    if(_fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_fileHandle);
        _fileHandle = INVALID_HANDLE_VALUE;
    }
}
}// namespace ipc

// module ipc.shared_memory.windows;
