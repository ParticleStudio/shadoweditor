module;

#include "common/platform.hpp"

module common.shared_library;

#if defined(PLATFORM_OS_FAMILY_UNIX)

import common.exception;

#include <dlfcn.h>

#include <mutex>
#include <string>

#include "common/shared_library.h"

namespace common {
SharedLibrary::SharedLibrary() {
}

void SharedLibrary::Load(const std::string &rPath, int32_t flags) {
    std::scoped_lock<std::mutex> const lock(m_mutex);

    if(m_pHandle != nullptr) {
        throw RuntimeError("Library already loaded: " + rPath);
    }

    m_pHandle = dlopen(rPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if(m_pHandle == nullptr) {
        const char *err = dlerror();
        throw RuntimeError("Could not load library: " + (err ? std::string(err) : rPath));
    }
    m_path = rPath;
}

void SharedLibrary::Unload() {
    std::scoped_lock<std::mutex> const lock(m_mutex);

    if(m_pHandle != nullptr) {
        dlclose(m_pHandle);
        m_pHandle = nullptr;
    }
}

bool SharedLibrary::IsLoaded() const {
    return m_pHandle != nullptr;
}

const std::string &SharedLibrary::GetPath() const {
    return m_path;
}

std::string SharedLibrary::Prefix() {
#    if PLATFORM_OS == PLATFORM_OS_CYGWIN
    return "cyg";
#    else
    return "lib";
#    endif
}

std::string SharedLibrary::Suffix() {
#    if PLATFORM_OS == PLATFORM_OS_MAC_OS_X
#        if defined(_DEBUG) and !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
    return "d.dylib";
#        else
    return ".dylib";
#        endif
#    elif PLATFORM_OS == PLATFORM_OS_HPUX
#        if defined(_DEBUG) and !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
    return "d.sl";
#        else
    return ".sl";
#        endif
#    elif PLATFORM_OS == PLATFORM_OS_CYGWIN
#        if defined(_DEBUG) and !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
    return "d.dll";
#        else
    return ".dll";
#        endif
#    else
#        if defined(_DEBUG) and !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
    return "d.so";
#        else
    return ".so";
#        endif
#    endif
}

void *SharedLibrary::findSymbol(const std::string &rName) {
    std::scoped_lock<std::mutex> const lock(m_mutex);

    void *result = nullptr;
    if(m_pHandle) {
        result = dlsym(m_pHandle, name.c_str());
    }
    return result;
}

}// namespace common

#endif// #if defined(PLATFORM_OS_FAMILY_UNIX)

// module common.shared_library;
// module;
