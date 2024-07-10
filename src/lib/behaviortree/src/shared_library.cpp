#include "behaviortree/util/shared_library.h"

#include "behaviortree/exceptions.h"

behaviortree::SharedLibrary::SharedLibrary(const std::string& refPath, int flags) {
    Load(refPath, flags);
}

void* behaviortree::SharedLibrary::GetSymbol(const std::string& refName) {
    void* ptrResult = FindSymbol(refName);
    if(ptrResult != nullptr)
        return ptrResult;
    else
        throw RuntimeError("[SharedLibrary::getSymbol]: can't find symbol ", refName);
}

bool behaviortree::SharedLibrary::HasSymbol(const std::string& refName) {
    return FindSymbol(refName) != nullptr;
}

std::string behaviortree::SharedLibrary::GetOSName(const std::string& refName) {
    return Prefix() + refName + Suffix();
}
