#ifndef SHADOW_BEHAVIORTREE_COMMON_H
#define SHADOW_BEHAVIORTREE_COMMON_H

import shadow.platform;

#if defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(BEHAVIORTREE_SHARED_LIB)
#        if defined(BEHAVIORTREE_EXPORT)
#            define BEHAVIORTREE_API __declspec(dllexport)
#        else// !BEHAVIORTREE_EXPORT
#            define BEHAVIORTREE_API __declspec(dllimport)
#        endif// #if defined(BEHAVIORTREE_EXPORT)
#    else
#        define BEHAVIORTREE_API
#    endif
#else// !defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(BEHAVIORTREE_SHARED_LIB)
#        if defined(BEHAVIORTREE_EXPORT)
#            define BEHAVIORTREE_API __attribute__((visibility("default")))
#        else// !BEHAVIORTREE_EXPORT
#            define BEHAVIORTREE_API
#        endif// #if defined(BEHAVIORTREE_EXPORT)
#    else
#        define BEHAVIORTREE_API
#    endif
#endif// #if defined(PLATFORM_OS_FAMILY_WINDOWS)

#endif// SHADOW_BEHAVIORTREE_COMMON_H
