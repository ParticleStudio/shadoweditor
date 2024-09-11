#ifndef JSENGINE_COMMON_H
#define JSENGINE_COMMON_H

#include "common/platform.hpp"

#if defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(JSENGINE_SHARED_LIB)
#        if defined(JSENGINE_EXPORTS)
#            define JSENGINE_API __declspec(dllexport)
#        else// !JSENGINE_EXPORTS
#            define JSENGINE_API __declspec(dllimport)
#        endif// #if defined(JSENGINE_EXPORTS)
#    else
#        define JSENGINE_API
#    endif
#else// !defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(JSENGINE_SHARED_LIB)
#        if defined(JSENGINE_EXPORTS)
#            define JSENGINE_API __attribute__((visibility("default")))
#        else// !JSENGINE_EXPORTS
#            define JSENGINE_API
#        endif// #if defined(JSENGINE_EXPORTS)
#    else
#        define JSENGINE_API
#    endif
#endif// #if defined(PLATFORM_OS_FAMILY_WINDOWS)

#endif// JSENGINE_COMMON_H