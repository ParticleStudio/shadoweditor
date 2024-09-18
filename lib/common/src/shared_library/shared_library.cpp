module;

module common.shared_library;

import common.exception;

namespace common {
SharedLibrary::SharedLibrary(const std::string &rPath, int32_t flags) {
    Load(rPath, flags);
}

void *SharedLibrary::GetSymbol(const std::string &rSymbolName) {
    void *pResult = FindSymbol(rSymbolName);
    if(pResult != nullptr) {
        return pResult;
    }

    throw util::RuntimeError("[SharedLibrary::GetSymbol]: can't find symbol ", rSymbolName);
}

bool SharedLibrary::HasSymbol(const std::string &rSymbolName) {
    return FindSymbol(rSymbolName) != nullptr;
}

std::string SharedLibrary::GetOSName(const std::string &rName) {
    return Prefix() + rName + Suffix();
}
}// namespace common

// common.shared_library;
// module;
