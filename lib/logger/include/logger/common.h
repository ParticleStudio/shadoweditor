#ifndef LOGGER_COMMON_H
#define LOGGER_COMMON_H

#include "common/platform.hpp"

#if defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(LOGGER_SHARED_LIB)
#        if defined(LOGGER_EXPORT)
#            define LOGGER_API __declspec(dllexport)
#        else// !LOGGER_EXPORT
#            define LOGGER_API __declspec(dllimport)
#        endif// #if defined(LOGGER_EXPORT)
#    else
#        define LOGGER_API
#    endif
#else// !defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(LOGGER_SHARED_LIB)
#        if defined(LOGGER_EXPORT)
#            define LOGGER_API __attribute__((visibility("default")))
#        else// !LOGGER_EXPORT
#            define LOGGER_API
#        endif// #if defined(LOGGER_EXPORT)
#    else
#        define LOGGER_API
#    endif
#endif// #if defined(PLATFORM_OS_FAMILY_WINDOWS)

#endif// LOGGER_COMMON_H
