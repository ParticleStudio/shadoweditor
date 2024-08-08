#ifndef NET_DEFINE_H
#define NET_DEFINE_H

#include "common/platform.hpp"

#if defined(PLATFORM_OS_FAMILY_WINDOWS)

#    define _WIN32_WINNT 0x0501
#    include <Windows.h>

#endif

#endif// NET_DEFINE_H
