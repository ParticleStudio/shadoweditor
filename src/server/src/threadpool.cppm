export;

export module threadpool;

import common.threadpool;

#include "common/singleton.hpp"

export class ThreadPool final: public common::ThreadPool, public common::Singleton<ThreadPool> {
};
