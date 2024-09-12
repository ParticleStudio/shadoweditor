export;

export module threadpool;

import common.singleton;
import common.threadpool;

export class ThreadPool final: public common::ThreadPool, public common::Singleton<ThreadPool> {
};
