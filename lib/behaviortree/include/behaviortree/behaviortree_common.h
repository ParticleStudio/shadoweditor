#ifndef BEHAVIORTREE_BEHAVIORTREE_COMMON_H
#define BEHAVIORTREE_BEHAVIORTREE_COMMON_H

#include "common/platform.hpp"

#if defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(BEHAVIORTREE_SHARED_LIB)
#        if defined(BEHAVIORTREE_EXPORTS)
#            define BEHAVIORTREE_API __declspec(dllexport)
#        else// !BEHAVIORTREE_EXPORTS
#            define BEHAVIORTREE_API __declspec(dllimport)
#        endif// #if defined(BEHAVIORTREE_EXPORTS)
#    else
#        define BEHAVIORTREE_API
#    endif
#else// !defined(PLATFORM_OS_FAMILY_WINDOWS)
#    if defined(BEHAVIORTREE_SHARED_LIB)
#        if defined(BEHAVIORTREE_EXPORTS)
#            define BEHAVIORTREE_API __attribute__((visibility("default")))
#        else// !BEHAVIORTREE_EXPORTS
#            define BEHAVIORTREE_API
#        endif// #if defined(BEHAVIORTREE_EXPORTS)
#    else
#        define BEHAVIORTREE_API
#    endif
#endif// #if defined(PLATFORM_OS_FAMILY_WINDOWS)

#endif// BEHAVIORTREE_BEHAVIORTREE_COMMON_H
