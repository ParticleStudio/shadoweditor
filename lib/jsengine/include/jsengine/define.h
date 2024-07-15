#ifndef JSENGINE_DEFINE_H
#define JSENGINE_DEFINE_H

#ifdef SHARED_LIB
#    ifdef WIN32
#        ifdef DLLEXPORT
#            define JSENGINE_API __declspec(dllexport)
#        else
#            define JSENGINE_API __declspec(dllimport)
#        endif// !DLLEXPORT
#    else
#        define JSENGINE_API
#    endif// !WIN32
#else
#    define JSENGINE_API
#endif// !SHARED_LIB

#endif// !JSENGINE_DEFINE_H