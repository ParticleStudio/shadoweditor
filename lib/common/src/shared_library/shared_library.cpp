module;

#include <string>

module shadow.library;

import shadow.exception;

namespace shadow::library {
void *SharedLibrary::GetSymbol(const std::string &rSymbolName) {
    void *pResult = findSymbol(rSymbolName);
    if(pResult != nullptr) {
        return pResult;
    }

    throw shadow::exception::RuntimeError("[SharedLibrary::GetSymbol]: can't find symbol ", rSymbolName);
}

bool SharedLibrary::HasSymbol(const std::string &rSymbolName) {
    return findSymbol(rSymbolName) != nullptr;
}

std::string SharedLibrary::GetOSName(const std::string &rName) {
    return Prefix() + rName + Suffix();
}
} // namespace shadow::library

// module shadow.library;
// module;
