#include "behaviortree/util/shared_library.h"

#include "behaviortree/exceptions.h"

behaviortree::SharedLibrary::SharedLibrary(const std::string &rPath, int32_t flags) {
    Load(rPath, flags);
}

void *behaviortree::SharedLibrary::GetSymbol(const std::string &rName) {
    void *pResult = FindSymbol(rName);
    if(pResult != nullptr) {
        return pResult;
    } else {
        throw RuntimeError("[SharedLibrary::getSymbol]: can't find symbol ", rName);
    }
}

bool behaviortree::SharedLibrary::HasSymbol(const std::string &rName) {
    return FindSymbol(rName) != nullptr;
}

std::string behaviortree::SharedLibrary::GetOSName(const std::string &rName) {
    return Prefix() + rName + Suffix();
}
