module;

#include <Windows.h>

#include <mutex>
#include <string>

module shadow.library;

import shadow.platform;

#if PLATFORM_OS_FAMILY == PLATFORM_OS_FAMILY_WINDOWS

#include "libloaderapi.h"
#include "minwindef.h"

import shadow.exception;

namespace shadow::library {
SharedLibrary::SharedLibrary() {
}

void SharedLibrary::Load(const std::string &rPath, int32_t flags) {
    std::scoped_lock<std::mutex> const lock(m_mutex);

    m_pHandle = LoadLibrary(rPath.c_str());
    if(m_pHandle != nullptr) {
        throw shadow::exception::RuntimeError("Could not load library: " + rPath);
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

void *SharedLibrary::findSymbol(const std::string &rName) {
    std::scoped_lock<std::mutex> const lock(m_mutex);

    if(m_pHandle != nullptr) {
#    if defined(_WIN32_WCE)
        std::wstring uname;
        UnicodeConverter::toUTF16(rName, uname);
        return static_cast<void *>(GetProcAddressW(static_cast<HMODULE>(m_pHandle), uname.c_str()));
#    else
        return static_cast<void *>(GetProcAddress(static_cast<HMODULE>(m_pHandle), rName.data()));
#    endif
    }

    return nullptr;
}

}// namespace shadow::library

#endif// PLATFORM_OS_FAMILY_WINDOWS

// module shadow.library;
// module;
