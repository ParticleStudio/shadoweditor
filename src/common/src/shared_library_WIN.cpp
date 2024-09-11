#include "common/platform.hpp"

#ifdef PLATFORM_OS_FAMILY_WINDOWS

#include <Windows.h>

#include <mutex>
#include <string>

#include "common/exceptions.h"
#include "common/shared_library.h"
#include "libloaderapi.h"
#include "minwindef.h"

namespace util {
SharedLibrary::SharedLibrary() {
}

void SharedLibrary::Load(const std::string &rPath, int32_t flags) {
    std::scoped_lock<std::mutex> const lock(m_mutex);

    m_pHandle = LoadLibrary(rPath.c_str());
    if(m_pHandle != nullptr) {
        throw RuntimeError("Could not load library: " + rPath);
    }
    m_path = rPath;
}

void SharedLibrary::Unload() {
    std::scoped_lock<std::mutex> const lock(m_mutex);

    if(m_pHandle != nullptr) {
        FreeLibrary(static_cast<HMODULE>(m_pHandle));
        m_pHandle = nullptr;
    }
    m_path.clear();
}

bool SharedLibrary::IsLoaded() const {
    return m_pHandle != nullptr;
}

void *SharedLibrary::FindSymbol(const std::string &rName) {
    std::scoped_lock<std::mutex> const lock(m_mutex);

    if(m_pHandle != nullptr) {
#    if defined(_WIN32_WCE)
        std::wstring uname;
        UnicodeConverter::toUTF16(rName, uname);
        return static_cast<void *>(GetProcAddressW(static_cast<HMODULE>(m_pHandle), uname.c_str()));
#    else
        return static_cast<void *>(GetProcAddress(static_cast<HMODULE>(m_pHandle), rName.c_str()));
#    endif
    }

    return nullptr;
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

#endif// PLATFORM_OS_FAMILY_WINDOWS
