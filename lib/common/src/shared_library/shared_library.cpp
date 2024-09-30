module;

module common.shared_library;

import common.exception;

namespace common {
void *SharedLibrary::GetSymbol(const std::string &rSymbolName) {
    void *pResult = findSymbol(rSymbolName);
    if(pResult != nullptr) {
        return pResult;
    }

    throw util::RuntimeError("[SharedLibrary::GetSymbol]: can't find symbol ", rSymbolName);
}

bool SharedLibrary::HasSymbol(const std::string &rSymbolName) {
    return findSymbol(rSymbolName) != nullptr;
}

std::string SharedLibrary::GetOSName(const std::string &rName) {
    return Prefix() + rName + Suffix();
}
} // namespace common

// common.shared_library;
// module;
