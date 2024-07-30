#include "common/shared_library.h"

#include "common/exceptions.h"

namespace util {
SharedLibrary::SharedLibrary(const std::string &rPath, int32_t flags) {
    Load(rPath, flags);
}

void *SharedLibrary::GetSymbol(const std::string &rName) {
    void *pResult = FindSymbol(rName);
    if(pResult != nullptr) {
        return pResult;
    }

    throw util::RuntimeError("[SharedLibrary::GetSymbol]: can't find symbol ", rName);
}

bool SharedLibrary::HasSymbol(const std::string &rName) {
    return FindSymbol(rName) != nullptr;
}

std::string SharedLibrary::GetOSName(const std::string &rName) {
    return Prefix() + rName + Suffix();
}
}// namespace util
