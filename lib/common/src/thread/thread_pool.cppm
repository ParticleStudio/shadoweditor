/**
 * ██████  ███████       ████████ ██   ██ ██████  ███████  █████  ██████          ██████   ██████   ██████  ██
 * ██   ██ ██      ██ ██    ██    ██   ██ ██   ██ ██      ██   ██ ██   ██         ██   ██ ██    ██ ██    ██ ██
 * ██████  ███████          ██    ███████ ██████  █████   ███████ ██   ██         ██████  ██    ██ ██    ██ ██
 * ██   ██      ██ ██ ██    ██    ██   ██ ██   ██ ██      ██   ██ ██   ██         ██      ██    ██ ██    ██ ██
 * ██████  ███████          ██    ██   ██ ██   ██ ███████ ██   ██ ██████  ███████ ██       ██████   ██████  ███████
 *
 * @file BS.thread_pool.cppm
 * @author Barak Shoshany (baraksh@gmail.com) (https://baraksh.com/)
 * @version 5.0.0
 * @date 2024-12-19
 * @copyright Copyright (c) 2024 Barak Shoshany. Licensed under the MIT license. If you found this project useful, please consider starring it on GitHub! If you use this library in software of any kind, please provide a link to the GitHub repository https://github.com/bshoshany/thread-pool in the source code and documentation. If you use this library in published research, please cite it as follows: Barak Shoshany, "A C++17 Thread Pool for High-Performance Scientific Computing", doi:10.1016/j.softx.2024.101687, SoftwareX 26 (2024) 101687, arXiv:2105.00613
 *
 * @brief `BS::thread_pool`: a fast, lightweight, modern, and easy-to-use C++17/C++20/C++23 thread pool library. This module file wraps the header file BS_thread_pool.hpp inside a C++20 module so it can be imported using `import BS.thread_pool`.
 */

module;

// A macro indicating to the library that it is being imported as a module, as well as the version of the module file, which must match the version of the header file.
#define BS_THREAD_POOL_MODULE 5, 0, 0

#include "common/thread_pool.hpp"

export module shadow.thread.pool;

export namespace shadow::thread {
using shadow::thread::binary_semaphore;
using shadow::thread::common_index_type_t;
using shadow::thread::counting_semaphore;
using shadow::thread::light_thread_pool;
using shadow::thread::multi_future;
using shadow::thread::pause_thread_pool;
using shadow::thread::pr;
using shadow::thread::priority_t;
using shadow::thread::priority_thread_pool;
using shadow::thread::synced_stream;
using shadow::thread::this_thread;
using shadow::thread::Pool;
using shadow::thread::thread_pool_import_std;
using shadow::thread::thread_pool_module;
using shadow::thread::thread_pool_native_extensions;
using shadow::thread::thread_pool_version;
using shadow::thread::tp;
using shadow::thread::version;
using shadow::thread::wait_deadlock;
using shadow::thread::wdc_thread_pool;

#ifdef BS_THREAD_POOL_NATIVE_EXTENSIONS
using BS::get_os_process_affinity;
using BS::get_os_process_priority;
using BS::os_process_priority;
using BS::os_thread_priority;
using BS::set_os_process_affinity;
using BS::set_os_process_priority;
#endif
} // namespace shadow::thread