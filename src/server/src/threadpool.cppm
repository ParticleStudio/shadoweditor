module;

//#include <ctime>
#include "common/singleton.hpp"

export module server.threadpool;

import common.threadpool;

namespace server {
export class ThreadPool final: public common::ThreadPool, public common::Singleton<ThreadPool> {
};
}// namespace server

// module threadpool;
// module;
