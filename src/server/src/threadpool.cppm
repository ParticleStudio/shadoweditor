module;

#include <ctime>

export module server.threadpool;

import common.threadpool;

#include "common/singleton.hpp"

namespace server {
export class ThreadPool final: public common::ThreadPool, public common::Singleton<ThreadPool> {
};
}// namespace server

// module threadpool;
// module;
