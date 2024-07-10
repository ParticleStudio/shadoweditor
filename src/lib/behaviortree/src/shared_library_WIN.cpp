#include <Windows.h>

#include <mutex>
#include <string>

#include "behaviortree/exceptions.h"
#include "behaviortree/util/shared_library.h"

namespace behaviortree {
SharedLibrary::SharedLibrary() {
    m_Handle = nullptr;
}

void SharedLibrary::Load(const std::string &refPath, int) {
    std::unique_lock<std::mutex> lock(m_Mutex);

    m_Handle = LoadLibrary(refPath.c_str());
    if(!m_Handle) {
        throw RuntimeError("Could not load library: " + refPath);
    }
    m_Path = refPath;
}

void SharedLibrary::Unload() {
    std::unique_lock<std::mutex> lock(m_Mutex);

    if(m_Handle) {
        FreeLibrary((HMODULE)m_Handle);
        m_Handle = 0;
    }
    m_Path.clear();
}

bool SharedLibrary::IsLoaded() const {
    return m_Handle != nullptr;
}

void *SharedLibrary::FindSymbol(const std::string &name) {
    std::unique_lock<std::mutex> lock(m_Mutex);

    if(m_Handle) {
#if defined(_WIN32_WCE)
        std::wstring uname;
        UnicodeConverter::toUTF16(name, uname);
        return (void *)GetProcAddressW((HMODULE)m_Handle, uname.c_str());
#else
        return (void *)GetProcAddress((HMODULE)m_Handle, name.c_str());
#endif
    }

    return 0;
}

const std::string &SharedLibrary::GetPath() const {
    return m_Path;
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
