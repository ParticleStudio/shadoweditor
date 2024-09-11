export;

#include "common/singleton.h"
#include "common/threadpool.hpp"

export module threadpool;

export class ThreadPool final: public common::ThreadPool, public common::Singleton<ThreadPool> {
};
