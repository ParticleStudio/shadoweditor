#ifndef SHADOW_IPC_COMMON_H
#define SHADOW_IPC_COMMON_H

import shadow.platform;

#if defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(IPC_SHARED_LIB)
#        if defined(IPC_EXPORT)
#            define IPC_API __declspec(dllexport)
#        else // !IPC_EXPORT
#            define IPC_API __declspec(dllimport)
#        endif // #if defined(IPC_EXPORT)
#    else
#        define IPC_API
#    endif // #if defined(IPC_SHARED_LIB)
#else      // !defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(IPC_SHARED_LIB)
#        if defined(IPC_EXPORT)
#            define IPC_API __attribute__((visibility("default")))
#        else // !IPC_EXPORT
#            define IPC_API
#        endif // #if defined(IPC_EXPORT)
#    else
#        define IPC_API
#    endif // #if defined(IPC_SHARED_LIB)
#endif     // #if defined(PLATFORM_OS_FAMILY_WINDOWS)

#endif // SHADOW_IPC_COMMON_H
