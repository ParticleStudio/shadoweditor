#ifndef BEHAVIORTREE_DEFINE_H
#define BEHAVIORTREE_DEFINE_H

#ifdef SHARED_LIB
#    ifdef WIN32
#        ifdef DLLEXPORT
#            define BEHAVIORTREE_API __declspec(dllexport)
#        else
#            define BEHAVIORTREE_API __declspec(dllimport)
#        endif// !DLLEXPORT
#    else
#        define BEHAVIORTREE_API
#    endif// !WIN32
#else
#    define BEHAVIORTREE_API
#endif// !SHARED_LIB

#endif// BEHAVIORTREE_DEFINE_H
