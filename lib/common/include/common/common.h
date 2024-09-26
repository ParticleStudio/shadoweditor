#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

#include "common/platform.hpp"

#if defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(COMMON_SHARED_LIB)
#        if defined(COMMON_EXPORT)
#            define COMMON_API __declspec(dllexport)
#        else// !COMMON_EXPORT
#            define COMMON_API __declspec(dllimport)
#        endif// #if defined(COMMON_EXPORT)
#    else
#        define COMMON_API
#    endif
#else// !defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(COMMON_SHARED_LIB)
#        if defined(COMMON_EXPORT)
#            define COMMON_API __attribute__((visibility("default")))
#        else// !COMMON_EXPORT
#            define COMMON_API
#        endif// #if defined(COMMON_EXPORT)
#    else
#        define COMMON_API
#    endif
#endif// #if defined(PLATFORM_OS_FAMILY_WINDOWS)

#endif//COMMON_COMMON_H
