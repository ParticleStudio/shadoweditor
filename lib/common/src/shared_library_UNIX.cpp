#include "common/platform.hpp"

#ifdef PLATFORM_OS_FAMILY_UNIX

#    include <dlfcn.h>

#    include <mutex>
#    include <string>

#    include "common/exceptions.h"
#    include "common/shared_library.h"

namespace util {
SharedLibrary::SharedLibrary() {
    m_pHandle = nullptr;
}

void SharedLibrary::Load(const std::string &path, int) {
    std::unique_lock<std::mutex> lock(m_mutex);

    if(m_pHandle) {
        throw RuntimeError("Library already loaded: " + path);
    }

    m_pHandle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if(!m_pHandle) {
        const char *err = dlerror();
        throw RuntimeError("Could not load library: " + (err ? std::string(err) : path));
    }
    m_path = path;
}

void SharedLibrary::Unload() {
    std::unique_lock<std::mutex> lock(m_mutex);

    if(m_pHandle) {
        dlclose(m_pHandle);
        m_pHandle = nullptr;
    }
}

bool SharedLibrary::IsLoaded() const {
    return m_pHandle != nullptr;
}

void *SharedLibrary::FindSymbol(const std::string &name) {
    std::unique_lock<std::mutex> lock(m_mutex);

    void *result = nullptr;
    if(m_pHandle) {
        result = dlsym(m_pHandle, name.c_str());
    }
    return result;
}

const std::string &SharedLibrary::GetPath() const {
    return m_path;
}

std::string SharedLibrary::Prefix() {
#    if BT_OS == BT_OS_CYGWIN
    return "cyg";
#    else
    return "lib";
#    endif
}

std::string SharedLibrary::Suffix() {
#    if BT_OS == BT_OS_MAC_OS_X
#        if defined(_DEBUG) && !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
    return "d.dylib";
#        else
    return ".dylib";
#        endif
#    elif BT_OS == BT_OS_HPUX
#        if defined(_DEBUG) && !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
    return "d.sl";
#        else
    return ".sl";
#        endif
#    elif BT_OS == BT_OS_CYGWIN
#        if defined(_DEBUG) && !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
    return "d.dll";
#        else
    return ".dll";
#        endif
#    else
#        if defined(_DEBUG) && !defined(CL_NO_SHARED_LIBRARY_DEBUG_SUFFIX)
    return "d.so";
#        else
    return ".so";
#        endif
#    endif
}

}// namespace util

#endif// SHADOW_OS_FAMILY_UNIX
