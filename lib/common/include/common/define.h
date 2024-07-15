#ifndef COMMON_DEFINE_H
#define COMMON_DEFINE_H

#ifdef SHARED_LIB
#    ifdef WIN32
#        ifdef DLLEXPORT
#            define COMMON_API __declspec(dllexport)
#        else
#            define COMMON_API __declspec(dllimport)
#        endif// !DLLEXPORT
#    else
#        define COMMON_API
#    endif// !WIN32
#else
#    define COMMON_API
#endif// !SHARED_LIB

#endif//COMMON_DEFINE_H
