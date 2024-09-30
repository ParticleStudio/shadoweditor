#ifndef TEXT_COMMON_H
#define TEXT_COMMON_H

#include "common/platform.hpp"

#if defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(TEXT_SHARED_LIB)
#        if defined(TEXT_EXPORT)
#            define TEXT_API __declspec(dllexport)
#        else // !TEXT_EXPORT
#            define TEXT_API __declspec(dllimport)
#        endif // #if defined(TEXT_EXPORT)
#    else
#        define TEXT_API
#    endif
#else // !defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(TEXT_SHARED_LIB)
#        if defined(TEXT_EXPORT)
#            define TEXT_API __attribute__((visibility("default")))
#        else // !TEXT_EXPORT
#            define TEXT_API
#        endif // #if defined(TEXT_EXPORT)
#    else
#        define TEXT_API
#    endif
#endif // #if defined(PLATFORM_OS_FAMILY_WINDOWS)

#endif //TEXT_COMMON_H
