#include "common/platform.hpp"

#ifdef SHADOW_OS_FAMILY_WINDOWS

#include <Windows.h>

#include <mutex>
#include <string>

#include "common/exceptions.h"
#include "common/shared_library.h"

namespace util {
SharedLibrary::SharedLibrary() {
    m_pHandle = nullptr;
}

void SharedLibrary::Load(const std::string &rPath, int32_t) {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_pHandle = LoadLibrary(rPath.c_str());
    if(!m_pHandle) {
        throw RuntimeError("Could not load library: " + rPath);
    }
    m_path = rPath;
}

void SharedLibrary::Unload() {
    std::unique_lock<std::mutex> lock(m_mutex);

    if(m_pHandle) {
        FreeLibrary((HMODULE)m_pHandle);
        m_pHandle = 0;
    }
    m_path.clear();
}

bool SharedLibrary::IsLoaded() const {
    return m_pHandle != nullptr;
}

void *SharedLibrary::FindSymbol(const std::string &name) {
    std::unique_lock<std::mutex> lock(m_mutex);

    if(m_pHandle) {
#    if defined(_WIN32_WCE)
        std::wstring uname;
        UnicodeConverter::toUTF16(name, uname);
        return (void *)GetProcAddressW((HMODULE)m_Handle, uname.c_str());
#    else
        return (void *)GetProcAddress((HMODULE)m_pHandle, name.c_str());
#    endif
    }

    return 0;
}

const std::string &SharedLibrary::GetPath() const {
    return m_path;
}

std::string SharedLibrary::Prefix() {
    return "";
}

std::string SharedLibrary::Suffix() {
#    if defined(_DEBUG)
    return "d.dll";
#    else
    return ".dll";
#    endif
}

}// namespace util

#endif// SHADOW_OS_FAMILY_UNIX
