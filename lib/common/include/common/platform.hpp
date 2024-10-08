#ifndef COMMON_PLATFORM_HPP
#define COMMON_PLATFORM_HPP

#include <cstdint>

//
// Platform Identification
//
static constexpr const uint16_t PLATFORM_OS_FREE_BSD = 0x0001;
static constexpr const uint16_t PLATFORM_OS_AIX = 0x0002;
static constexpr const uint16_t PLATFORM_OS_HPUX = 0x0003;
static constexpr const uint16_t PLATFORM_OS_TRU64 = 0x0004;
static constexpr const uint16_t PLATFORM_OS_LINUX = 0x0005;
static constexpr const uint16_t PLATFORM_OS_MAC_OS_X = 0x0006;
static constexpr const uint16_t PLATFORM_OS_NET_BSD = 0x0007;
static constexpr const uint16_t PLATFORM_OS_OPEN_BSD = 0x0008;
static constexpr const uint16_t PLATFORM_OS_IRIX = 0x0009;
static constexpr const uint16_t PLATFORM_OS_SOLARIS = 0x000a;
static constexpr const uint16_t PLATFORM_OS_QNX = 0x000b;
static constexpr const uint16_t PLATFORM_OS_VXWORKS = 0x000c;
static constexpr const uint16_t PLATFORM_OS_CYGWIN = 0x000d;
static constexpr const uint16_t PLATFORM_OS_NACL = 0x000e;
static constexpr const uint16_t PLATFORM_OS_ANDROID = 0x000f;
static constexpr const uint16_t PLATFORM_OS_UNKNOWN_UNIX = 0x00ff;
static constexpr const uint16_t PLATFORM_OS_WINDOWS_NT = 0x1001;
static constexpr const uint16_t PLATFORM_OS_WINDOWS_CE = 0x1011;
static constexpr const uint16_t PLATFORM_OS_VMS = 0x2001;

#if defined(__FreeBSD__) or defined(__FreeBSD_kernel__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS_FAMILY_BSD 1
#    define PLATFORM_OS PLATFORM_OS_FREE_BSD
#elif defined(_AIX) or defined(__TOS_AIX__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS PLATFORM_OS_AIX
#elif defined(hpux) or defined(_hpux) or defined(__hpux)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS PLATFORM_OS_HPUX
#elif defined(__digital__) or defined(__osf__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS PLATFORM_OS_TRU64
#elif defined(__NACL__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS PLATFORM_OS_NACL
#elif defined(linux) or defined(__linux) or defined(__linux__) or defined(__TOS_LINUX__) or defined(__EMSCRIPTEN__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    if defined(__ANDROID__)
#        define PLATFORM_OS PLATFORM_OS_ANDROID
#    else
#        define PLATFORM_OS PLATFORM_OS_LINUX
#    endif
#    if defined(__EMSCRIPTEN__)
#        define PLATFORM_EMSCRIPTEN
#    endif
#elif defined(__APPLE__) or defined(__TOS_MACOS__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS_FAMILY_BSD 1
#    define PLATFORM_OS PLATFORM_OS_MAC_OS_X
#elif defined(__NetBSD__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS_FAMILY_BSD 1
#    define PLATFORM_OS PLATFORM_OS_NET_BSD
#elif defined(__OpenBSD__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS_FAMILY_BSD 1
#    define PLATFORM_OS PLATFORM_OS_OPEN_BSD
#elif defined(sgi) or defined(__sgi)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS PLATFORM_OS_IRIX
#elif defined(sun) or defined(__sun)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS PLATFORM_OS_SOLARIS
#elif defined(__QNX__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS PLATFORM_OS_QNX
#elif defined(__CYGWIN__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS PLATFORM_OS_CYGWIN
#elif defined(PLATFORM_VXWORKS)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS PLATFORM_OS_VXWORKS
#elif defined(unix) or defined(__unix) or defined(__unix__)
#    define PLATFORM_OS_FAMILY_UNIX 1
#    define PLATFORM_OS PLATFORM_OS_UNKNOWN_UNIX
#elif defined(WIN32) or defined(_WIN32) or defined(WIN64) or defined(_WIN64)
#    define PLATFORM_OS_FAMILY_WINDOWS 1
#    define PLATFORM_OS PLATFORM_OS_WINDOWS_NT
#elif defined(__VMS)
#    define SHADOW_OS_FAMILY_VMS 1
#    define SHADOW_OS PLATFORM_OS_VMS
#endif


#if !defined(PLATFORM_OS)
#    error "Unknown Platform."
#endif


//
// Hardware Architecture and Byte Order
//
static constexpr const uint8_t PLATFORM_ARCH_ALPHA = 0x01;
static constexpr const uint8_t PLATFORM_ARCH_IA32 = 0x02;
static constexpr const uint8_t PLATFORM_ARCH_IA64 = 0x03;
static constexpr const uint8_t PLATFORM_ARCH_MIPS = 0x04;
static constexpr const uint8_t PLATFORM_ARCH_HPPA = 0x05;
static constexpr const uint8_t PLATFORM_ARCH_PPC = 0x06;
static constexpr const uint8_t PLATFORM_ARCH_POWER = 0x07;
static constexpr const uint8_t PLATFORM_ARCH_SPARC = 0x08;
static constexpr const uint8_t PLATFORM_ARCH_AMD64 = 0x09;
static constexpr const uint8_t PLATFORM_ARCH_ARM = 0x0a;
static constexpr const uint8_t PLATFORM_ARCH_M68K = 0x0b;
static constexpr const uint8_t PLATFORM_ARCH_S390 = 0x0c;
static constexpr const uint8_t PLATFORM_ARCH_SH = 0x0d;
static constexpr const uint8_t PLATFORM_ARCH_NIOS2 = 0x0e;
static constexpr const uint8_t PLATFORM_ARCH_AARCH64 = 0x0f;
static constexpr const uint8_t PLATFORM_ARCH_ARM64 = 0x0f; // same as PLATFORM_ARCH_AARCH64
static constexpr const uint8_t PLATFORM_ARCH_RISCV64 = 0x10;
static constexpr const uint8_t PLATFORM_ARCH_RISCV32 = 0x11;
static constexpr const uint8_t PLATFORM_ARCH_LOONGARCH64 = 0x12;


#if defined(__ALPHA) or defined(__alpha) or defined(__alpha__) or defined(_M_ALPHA)
#    define PLATFORM_ARCH PLATFORM_ARCH_ALPHA
#    define PLATFORM_ARCH_LITTLE_ENDIAN 1
#elif defined(i386) or defined(__i386) or defined(__i386__) or defined(_M_IX86) or defined(POCO_EMSCRIPTEN)
#    define PLATFORM_ARCH PLATFORM_ARCH_IA32
#    define PLATFORM_ARCH_LITTLE_ENDIAN 1
#elif defined(_IA64) or defined(__IA64__) or defined(__ia64__) or defined(__ia64) or defined(_M_IA64)
#    define PLATFORM_ARCH PLATFORM_ARCH_IA64
#    if defined(hpux) or defined(_hpux)
#        define PLATFORM_ARCH_BIG_ENDIAN 1
#    else
#        define PLATFORM_ARCH_LITTLE_ENDIAN 1
#    endif
#elif defined(__x86_64__) or defined(_M_X64)
#    define PLATFORM_ARCH PLATFORM_ARCH_AMD64
#    define PLATFORM_ARCH_LITTLE_ENDIAN 1
#elif defined(__mips__) or defined(__mips) or defined(__MIPS__) or defined(_M_MRX000)
#    define PLATFORM_ARCH PLATFORM_ARCH_MIPS
#    if defined(PLATFORM_OS_FAMILY_WINDOWS)
// Is this OK? Supports windows only little endian??
#        define PLATFORM_ARCH_LITTLE_ENDIAN 1
#    elif defined(__MIPSEB__) or defined(_MIPSEB) or defined(__MIPSEB)
#        define PLATFORM_ARCH_BIG_ENDIAN 1
#    elif defined(__MIPSEL__) or defined(_MIPSEL) or defined(__MIPSEL)
#        define PLATFORM_ARCH_LITTLE_ENDIAN 1
#    else
#        error "MIPS but neither MIPSEL nor MIPSEB?"
#    endif
#elif defined(__hppa) or defined(__hppa__)
#    define PLATFORM_ARCH PLATFORM_ARCH_HPPA
#    define PLATFORM_ARCH_BIG_ENDIAN 1
#elif defined(__PPC) or defined(__POWERPC__) or defined(__powerpc) or defined(__PPC__) or \
        defined(__powerpc__) or defined(__ppc__) or defined(__ppc) or defined(_ARCH_PPC) or defined(_M_PPC)
#    define PLATFORM_ARCH PLATFORM_ARCH_PPC
#    if defined(__BYTE_ORDER__) and (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#        define PLATFORM_ARCH_LITTLE_ENDIAN 1
#    else
#        define PLATFORM_ARCH_BIG_ENDIAN 1
#    endif
#elif defined(_POWER) or defined(_ARCH_PWR) or defined(_ARCH_PWR2) or defined(_ARCH_PWR3) or \
        defined(_ARCH_PWR4) or defined(__THW_RS6000)
#    define PLATFORM_ARCH PLATFORM_ARCH_POWER
#    define PLATFORM_ARCH_BIG_ENDIAN 1
#elif defined(__sparc__) or defined(__sparc) or defined(sparc)
#    define PLATFORM_ARCH PLATFORM_ARCH_SPARC
#    define PLATFORM_ARCH_BIG_ENDIAN 1
#elif defined(__arm__) or defined(__arm) or defined(ARM) or defined(_ARM_) or defined(__ARM__) or defined(_M_ARM)
#    define PLATFORM_ARCH PLATFORM_ARCH_ARM
#    if defined(__ARMEB__)
#        define PLATFORM_ARCH_BIG_ENDIAN 1
#    else
#        define PLATFORM_ARCH_LITTLE_ENDIAN 1
#    endif
#elif defined(__arm64__) or defined(__arm64) or defined(_M_ARM64)
#    define PLATFORM_ARCH PLATFORM_ARCH_ARM64
#    if defined(__ARMEB__)
#        define PLATFORM_ARCH_BIG_ENDIAN 1
#    elif defined(__BYTE_ORDER__) and defined(__ORDER_BIG_ENDIAN__) and __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#        define PLATFORM_ARCH_BIG_ENDIAN 1
#    else
#        define PLATFORM_ARCH_LITTLE_ENDIAN 1
#    endif
#elif defined(__m68k__)
#    define PLATFORM_ARCH PLATFORM_ARCH_M68K
#    define PLATFORM_ARCH_BIG_ENDIAN 1
#elif defined(__s390__)
#    define PLATFORM_ARCH PLATFORM_ARCH_S390
#    define PLATFORM_ARCH_BIG_ENDIAN 1
#elif defined(__sh__) or defined(__sh) or defined(SHx) or defined(_SHX_)
#    define PLATFORM_ARCH PLATFORM_ARCH_SH
#    if defined(__LITTLE_ENDIAN__)
#        define PLATFORM_ARCH_LITTLE_ENDIAN 1
#    else
#        define PLATFORM_ARCH_BIG_ENDIAN 1
#    endif
#elif defined(nios2) or defined(__nios2) or defined(__nios2__)
#    define PLATFORM_ARCH PLATFORM_ARCH_NIOS2
#    if defined(__nios2_little_endian) or defined(nios2_little_endian) or defined(__nios2_little_endian__)
#        define PLATFORM_ARCH_LITTLE_ENDIAN 1
#    else
#        define PLATFORM_ARCH_BIG_ENDIAN 1
#    endif
#elif defined(__AARCH64EL__)
#    define PLATFORM_ARCH PLATFORM_ARCH_AARCH64
#    define PLATFORM_ARCH_LITTLE_ENDIAN 1
#elif defined(__AARCH64EB__)
#    define PLATFORM_ARCH PLATFORM_ARCH_AARCH64
#    define PLATFORM_ARCH_BIG_ENDIAN 1
#elif defined(__riscv)
#    if(__riscv_xlen == 64)
#        define PLATFORM_ARCH PLATFORM_ARCH_RISCV64
#        define PLATFORM_ARCH_LITTLE_ENDIAN 1
#    elif(__riscv_xlen == 32)
#        define PLATFORM_ARCH PLATFORM_ARCH_RISCV32
#        define PLATFORM_ARCH_LITTLE_ENDIAN 1
#    endif
#elif defined(__loongarch64)
#    define PLATFORM_ARCH PLATFORM_ARCH_LOONGARCH64
#    define PLATFORM_ARCH_LITTLE_ENDIAN 1
#endif


#ifdef __SOFTFP__
#    define PLATFORM_NO_FPENVIRONMENT
#endif


#if defined(__clang__)
#    define PLATFORM_COMPILER_CLANG
#    define PLATFORM_HAVE_CXXABI_H
#elif defined(_MSC_VER)
#    define PLATFORM_COMPILER_MSVC
#elif defined(__GNUC__)
#    define PLATFORM_COMPILER_GCC
#    define PLATFORM_HAVE_CXXABI_H
#    if defined(__MINGW32__) or defined(__MINGW64__)
#        define PLATFORM_COMPILER_MINGW
#    endif
#elif defined(__MINGW32__) or defined(__MINGW64__)
#    define PLATFORM_COMPILER_MINGW
#elif defined(__INTEL_COMPILER) or defined(__ICC) or defined(__ECC) or defined(__ICL)
#    define PLATFORM_COMPILER_INTEL
#elif defined(__SUNPRO_CC)
#    define PLATFORM_COMPILER_SUN
#elif defined(__MWERKS__) or defined(__CWCC__)
#    define PLATFORM_COMPILER_CODEWARRIOR
#elif defined(__sgi) or defined(sgi)
#    define PLATFORM_COMPILER_SGI
#elif defined(__HP_aCC)
#    define PLATFORM_COMPILER_HP_ACC
#elif defined(__BORLANDC__) or defined(__CODEGEARC__)
#    define PLATFORM_COMPILER_CBUILDER
#elif defined(__DMC__)
#    define PLATFORM_COMPILER_DMARS
#elif defined(__DECCXX)
#    define PLATFORM_COMPILER_COMPAC
#elif(defined(__xlc__) or defined(__xlC__)) and defined(__IBMCPP__)
#    define PLATFORM_COMPILER_IBM_XLC // IBM XL C++
#elif defined(__IBMCPP__) and defined(__COMPILER_VER__)
#    define PLATFORM_COMPILER_IBM_XLC_ZOS // IBM z/OS C++
#endif


#ifdef __GNUC__
#    define PLATFORM_UNUSED __attribute__((unused))
#else
#    define PLATFORM_UNUSED
#endif // __GNUC__


#if !defined(PLATFORM_ARCH)
#    error "Unknown Hardware Architecture."
#endif


#if defined(PLATFORM_OS_FAMILY_WINDOWS)
#    define PLATFORM_DEFAULT_NEWLINE_CHARS "\r\n"
#else
#    define PLATFORM_DEFAULT_NEWLINE_CHARS "\n"
#endif


#endif // COMMON_PLATFORM_HPP
