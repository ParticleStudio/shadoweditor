#include <Windows.h>

#include <mutex>
#include <string>

#include "behaviortree/exceptions.h"
#include "behaviortree/util/shared_library.h"

namespace behaviortree {
SharedLibrary::SharedLibrary() {
    m_handle = nullptr;
}

void SharedLibrary::Load(const std::string &rPath, int32_t) {
    std::unique_lock<std::mutex> lock(m_mutex);

    m_handle = LoadLibrary(rPath.c_str());
    if(!m_handle) {
        throw RuntimeError("Could not load library: " + rPath);
    }
    m_path = rPath;
}

void SharedLibrary::Unload() {
    std::unique_lock<std::mutex> lock(m_mutex);

    if(m_handle) {
        FreeLibrary((HMODULE)m_handle);
        m_handle = 0;
    }
    m_path.clear();
}

bool SharedLibrary::IsLoaded() const {
    return m_handle != nullptr;
}

void *SharedLibrary::FindSymbol(const std::string &name) {
    std::unique_lock<std::mutex> lock(m_mutex);

    if(m_handle) {
#if defined(_WIN32_WCE)
        std::wstring uname;
        UnicodeConverter::toUTF16(name, uname);
        return (void *)GetProcAddressW((HMODULE)m_Handle, uname.c_str());
#else
        return (void *)GetProcAddress((HMODULE)m_handle, name.c_str());
#endif
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
#if defined(_DEBUG)
    return "d.dll";
#else
    return ".dll";
#endif
}

}// namespace behaviortree
