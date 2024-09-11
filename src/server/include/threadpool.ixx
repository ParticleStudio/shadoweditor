export;

#include "common/threadpool.hpp"

export module threadpool;

import common.singleton;

export class ThreadPool final: public common::ThreadPool, public common::Singleton<ThreadPool> {
};
