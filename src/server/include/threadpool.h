#ifndef SHADOWEDITOR_THREADPOOL_H
#define SHADOWEDITOR_THREADPOOL_H

#include "common/singleton.h"
#include "common/threadpool.hpp"

class ThreadPool final: public common::ThreadPool, public common::Singleton<ThreadPool> {
 public:
};

#endif//SHADOWEDITOR_THREADPOOL_H
