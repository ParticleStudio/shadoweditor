module;

export module common.threadpool;

#ifndef __cpp_exceptions
#    define THREADPOOL_DISABLE_EXCEPTION_HANDLING
#    undef THREADPOOL_ENABLE_WAIT_DEADLOCK_CHECK
#endif

import <chrono>;            // std::chrono
import <condition_variable>;// std::condition_variable
import <cstddef>;           // std::size_t

#ifdef THREADPOOL_ENABLE_PRIORITY
import <cstdint>;// std::int_least16_t
#endif

#ifndef THREADPOOL_DISABLE_EXCEPTION_HANDLING
import <exception>;// std::current_exception
#endif

import <functional>;// std::function
import <future>;    // std::future, std::future_status, std::promise
import <memory>;    // std::make_shared, std::make_unique, std::shared_ptr, std::unique_ptr
import <mutex>;     // std::mutex, std::scoped_lock, std::unique_lock
import <optional>;  // std::nullopt, std::optional
import <queue>;     // std::priority_queue (if priority enabled), std::queue

#ifdef THREADPOOL_ENABLE_WAIT_DEADLOCK_CHECK
import <stdexcept>;// std::runtime_error
#endif

import <array>;
import <thread>;     // std::thread
import <type_traits>;// std::conditional_t, std::decay_t, std::invoke_result_t, std::is_void_v, std::remove_const_t (if priority enabled)
import <utility>;    // std::forward, std::move
import <vector>;     // std::vector

#include "common/common.h"

/**
 * @brief A namespace used by Barak Shoshany's projects.
 */
namespace common {
export class ThreadPool;

/**
 * @brief A convenient shorthand for the type of `std::thread::hardware_concurrency()`. Should evaluate to unsigned int.
 */
using concurrency_t = std::invoke_result_t<decltype(std::thread::hardware_concurrency)>;

#ifdef THREADPOOL_ENABLE_PRIORITY
/**
 * @brief A type used to indicate the priority of a task. Defined to be an integer with a width of (at least) 16 bits.
 */
using priority_t = std::int_least16_t;

/**
 * @brief A namespace containing some pre-defined priorities for convenience.
 */
namespace pr {
constexpr priority_t highest = 32767;
constexpr priority_t high = 16383;
constexpr priority_t normal = 0;
constexpr priority_t low = -16384;
constexpr priority_t lowest = -32768;
}// namespace pr

// Macros used internally to enable or disable the priority arguments in the relevant functions.
#    define THREADPOOL_PRIORITY_INPUT , const priority_t priority = 0
#    define THREADPOOL_PRIORITY_OUTPUT , priority
#else
#    define THREADPOOL_PRIORITY_INPUT
#    define THREADPOOL_PRIORITY_OUTPUT
#endif

/**
 * @brief A namespace used to obtain information about the current thread.
 */
export namespace this_thread {
/**
 * @brief A type returned by `this_thread::get_index()` which can optionally contain the index of a thread, if that thread belongs to a `ThreadPool`. Otherwise, it will contain no value.
 */
using OptionalIndex = std::optional<std::size_t>;

/**
 * @brief A type returned by `this_thread::get_pool()` which can optionally contain the pointer to the pool that owns a thread, if that thread belongs to a `ThreadPool`. Otherwise, it will contain no value.
 */
using OptionalPool = std::optional<ThreadPool *>;

/**
 * @brief A helper class to store information about the index of the current thread.
 */
class [[nodiscard]] ThreadInfoIndex {
    friend class common::ThreadPool;

 public:
    /**
     * @brief Get the index of the current thread. If this thread belongs to a `ThreadPool` object, it will have an index from 0 to `ThreadPool::GetThreadNum() - 1`. Otherwise, for example if this thread is the main thread or an independent `std::thread`, `std::nullopt` will be returned.
     *
     * @return An `std::optional` object, optionally containing a thread index. Unless you are 100% sure this thread is in a pool, first use `std::optional::has_value()` to check if it contains a value, and if so, use `std::optional::value()` to obtain that value.
     */
    [[nodiscard]] OptionalIndex operator()() const {
        return m_index;
    }

 private:
    /**
     * @brief The index of the current thread.
     */
    OptionalIndex m_index = std::nullopt;
};// class ThreadInfoIndex

/**
 * @brief A helper class to store information about the thread pool that owns the current thread.
 */
class [[nodiscard]] ThreadInfoPool {
    friend class common::ThreadPool;

 public:
    /**
     * @brief Get the pointer to the thread pool that owns the current thread. If this thread belongs to a `common::ThreadPool` object, a pointer to that object will be returned. Otherwise, for example if this thread is the main thread or an independent `std::thread`, `std::nullopt` will be returned.
     *
     * @return An `std::optional` object, optionally containing a pointer to a thread pool. Unless you are 100% sure this thread is in a pool, first use `std::optional::has_value()` to check if it contains a value, and if so, use `std::optional::value()` to obtain that value.
     */
    [[nodiscard]] OptionalPool operator()() const;

 private:
    /**
     * @brief A pointer to the thread pool that owns the current thread.
     */
    OptionalPool m_pool = std::nullopt;

};// class ThreadInfoPool

/**
 * @brief A `thread_local` object used to obtain information about the index of the current thread.
 */
inline thread_local ThreadInfoIndex GetIndex;

/**
 * @brief A `thread_local` object used to obtain information about the thread pool that owns the current thread.
 */
inline thread_local ThreadInfoPool GetPool;
}// namespace this_thread

/**
 * @brief A helper class to facilitate waiting for and/or getting the results of multiple futures at once.
 *
 * @tparam T The return type of the futures.
 */
template<typename T>
class [[nodiscard]] MultiFuture: public std::vector<std::future<T>> {
 public:
    // Inherit all constructors from the base class `std::vector`.
    using std::vector<std::future<T>>::vector;

    // The copy constructor and copy assignment operator are deleted. The elements stored in a `MultiFuture` are futures, which cannot be copied.
    MultiFuture(const MultiFuture &) = delete;
    MultiFuture &operator=(const MultiFuture &) = delete;

    // The move constructor and move assignment operator are defaulted.
    MultiFuture(MultiFuture &&) = default;
    MultiFuture &operator=(MultiFuture &&) = default;

    /**
     * @brief Get the results from all the futures stored in this `MultiFuture`, rethrowing any stored exceptions.
     *
     * @return If the futures return `void`, this function returns `void` as well. Otherwise, it returns a vector containing the results.
     */
    [[nodiscard]] std::conditional_t<std::is_void_v<T>, void, std::vector<T>> Get() {
        if constexpr(std::is_void_v<T>) {
            for(std::future<T> &rFuture: *this) {
                rFuture.get();
            }
            return;
        } else {
            std::vector<T> resultVec;
            resultVec.reserve(this->size());
            for(std::future<T> &rFuture: *this) {
                resultVec.push_back(rFuture.get());
            }
            return resultVec;
        }
    }

    /**
     * @brief Check how many of the futures stored in this `MultiFuture` are ready.
     *
     * @return The number of ready futures.
     */
    [[nodiscard]] std::size_t ReadyNum() const {
        std::size_t num = 0;
        for(const std::future<T> &rFuture: *this) {
            if(rFuture.wait_for(std::chrono::duration<double>::zero()) == std::future_status::ready) {
                ++num;
            }
        }
        return num;
    }

    /**
     * @brief Check if all the futures stored in this `MultiFuture` are Valid.
     *
     * @return `true` if all futures are valid, `false` if at least one of the futures is not Valid.
     */
    [[nodiscard]] bool Valid() const {
        bool isValid = true;
        for(const std::future<T> &rFuture: *this) {
            isValid = isValid && rFuture.valid();
        }
        return isValid;
    }

    /**
     * @brief Wait for all the futures stored in this `MultiFuture`.
     */
    void Wait() const {
        for(const std::future<T> &rFuture: *this) {
            rFuture.wait();
        }
    }

    /**
     * @brief Wait for all the futures stored in this `multi_future`, but stop waiting after the specified duration has passed. This function first waits for the first future for the desired duration. If that future is ready before the duration expires, this function waits for the second future for whatever remains of the duration. It continues similarly until the duration expires.
     *
     * @tparam R An arithmetic type representing the number of ticks to Wait.
     * @tparam P An `std::ratio` representing the length of each tick in seconds.
     * @param rDuration The amount of time to Wait.
     * @return `true` if all futures have been waited for before the duration expired, `false` otherwise.
     */
    template<typename R, typename P>
    bool WaitFor(const std::chrono::duration<R, P> &rDuration) const {
        const std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::steady_clock::now();
        for(const std::future<T> &rFuture: *this) {
            rFuture.wait_for(rDuration - (std::chrono::steady_clock::now() - startTime));
            if(rDuration < std::chrono::steady_clock::now() - startTime) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Wait for all the futures stored in this `MultiFuture`, but stop waiting after the specified time point has been reached. This function first waits for the first future until the desired time point. If that future is ready before the time point is reached, this function waits for the second future until the desired time point. It continues similarly until the time point is reached.
     *
     * @tparam C The type of the clock used to measure time.
     * @tparam D An `std::chrono::duration` type used to indicate the time point.
     * @param rTimeoutTime The time point at which to stop waiting.
     * @return `true` if all futures have been waited for before the time point was reached, `false` otherwise.
     */
    template<typename C, typename D>
    bool WaitUntil(const std::chrono::time_point<C, D> &rTimeoutTime) const {
        for(const std::future<T> &rFuture: *this) {
            rFuture.wait_until(rTimeoutTime);
            if(rTimeoutTime < std::chrono::steady_clock::now()) {
                return false;
            }
        }
        return true;
    }
};// class MultiFuture

/**
 * @brief A fast, lightweight, and easy-to-use C++17 thread pool class.
 */
class [[nodiscard]] COMMON_API ThreadPool {
 public:
    /**
     * @brief Construct a new thread pool. The number of threads will be the total number of hardware threads available, as reported by the implementation. This is usually determined by the number of cores in the CPU. If a core is hyperthreaded, it will count as two threads.
     */
    ThreadPool() = default;

    // The copy and move constructors and assignment operators are deleted. The thread pool uses a mutex, which cannot be copied or moved.
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    /**
     * @brief Destruct the thread pool. Waits for all tasks to complete, then destroys all threads. Note that if the pool is paused, then any tasks still in the queue will never be executed.
     */
    virtual ~ThreadPool();

 public:
    virtual void Init();

    /**
     * @brief Construct a new thread pool with the specified number of threads.
     *
     * @param threadNum The number of threads to use.
     */
    virtual void Init(const concurrency_t);

    /**
     * @brief Construct a new thread pool with the specified number of threads and initialization function.
     *
     * @param threadNum The number of threads to use.
     * @param rInitTask An initialization function to run in each thread before it starts to execute any submitted tasks. The function must take no arguments and have no return value. It will only be executed exactly once, when the thread is first constructed.
     */
    virtual void Init(const concurrency_t threadNum, const std::function<void()> &rInitTask);

    /**
     * @brief Create the threads in the pool and assign a Run to each thread.
     *
     * @param rInitTask An initialization function to run in each thread before it starts to execute any submitted tasks.
     */
    virtual void Init(const std::function<void()> &rInitTask);

    /**
     * @brief Destroy the threads in the pool.
     */
    virtual void Release();

    virtual void JoinAll();

    /**
     * @brief Get a vector containing the underlying implementation-defined thread handles for each of the pool's threads, as obtained by `std::thread::native_handle()`. Only enabled if `THREADPOOL_ENABLE_NATIVE_HANDLES` is defined.
     *
     * @return The native thread handles.
     */
    [[nodiscard]] virtual std::vector<std::thread::native_handle_type> GetNativeHandle() const;

    /**
     * @brief Get the number of tasks currently waiting in the queue to be executed by the threads.
     *
     * @return The number of queued tasks.
     */
    [[nodiscard]] virtual std::size_t GetTaskQueued() const;

    /**
     * @brief Get the number of tasks currently being executed by the threads.
     *
     * @return The number of running tasks.
     */
    [[nodiscard]] virtual std::size_t GetRunningTaskNum() const;

    /**
     * @brief Get the total number of unfinished tasks: either still waiting in the queue, or running in a thread. Note that `GetTaskNum() == GetTaskQueued() + GetRunningTaskNum()`.
     *
     * @return The total number of tasks.
     */
    [[nodiscard]] virtual std::size_t GetTaskNum() const;

    /**
     * @brief Get the number of threads in the pool.
     *
     * @return The number of threads.
     */
    [[nodiscard]] virtual concurrency_t GetThreadNum() const;

    /**
     * @brief Get a vector containing the unique identifiers for each of the pool's threads, as obtained by `std::thread::get_id()`.
     *
     * @return The unique thread identifiers.
     */
    [[nodiscard]] virtual std::vector<std::thread::id> GetThreadIds() const;

    /**
     * @brief Check whether the pool is currently paused. Only enabled if `THREADPOOL_ENABLE_PAUSE` is defined.
     *
     * @return `true` if the pool is paused, `false` if it is not paused.
     */
    [[nodiscard]] virtual bool IsPaused() const;

    /**
     * @brief Pause the pool. The workers will temporarily stop retrieving new tasks out of the queue, although any tasks already executed will keep running until they are finished. Only enabled if `THREADPOOL_ENABLE_PAUSE` is defined.
     */
    virtual void Pause();

    /**
     * @brief Purge all the tasks waiting in the queue. Tasks that are currently running will not be affected, but any tasks still waiting in the queue will be discarded, and will never be executed by the threads. Please note that there is no way to restore the purged tasks.
     */
    virtual void Purge();

    /**
     * @brief Submit a function with no arguments and no return value into the task queue, with the specified priority. To push a function with arguments, enclose it in a lambda expression. Does not return a future, so the user must use `Wait()` or some other method to ensure that the task finishes executing, otherwise bad things will happen.
     *
     * @tparam Func The type of the function.
     * @param rTaskFunc The function to push.
     * @param priority The priority of the task. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `THREADPOOL_ENABLE_PRIORITY` is defined.
     */
    template<typename Func>
    void DetachTask(Func &&rTaskFunc THREADPOOL_PRIORITY_INPUT) {
        {
            const std::scoped_lock lock(m_taskMutex);
            m_taskQueue.emplace(std::forward<Func>(rTaskFunc) THREADPOOL_PRIORITY_OUTPUT);
        }
        m_taskAvailableCV.notify_one();
    }

    /**
     * @brief Parallelize a loop by automatically splitting it into blocks and submitting each block separately to the queue, with the specified priority. The block function takes two arguments, the start and end of the block, so that it is only called only once per block, but it is up to the user make sure the block function correctly deals with all the indices in each block. Does not return a `multi_future`, so the user must use `Wait()` or some other method to ensure that the loop finishes executing, otherwise bad things will happen.
     *
     * @tparam T The type of the indices. Should be a signed or unsigned integer.
     * @tparam Func The type of the function to loop through.
     * @param firstIndex The first index in the loop.
     * @param indexAfterLast The index after the last index in the loop. The loop will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no blocks will be submitted.
     * @param rBlockFunc A function that will be called once per block. Should take exactly two arguments: the first index in the block and the index after the last index in the block. `block(start, end)` should typically involve a loop of the form `for (T i = start; i < end; ++i)`.
     * @param blockNum The maximum number of blocks to split the loop into. The default is 0, which means the number of blocks will be equal to the number of threads in the pool.
     * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `THREADPOOL_ENABLE_PRIORITY` is defined.
     */
    template<typename T, typename Func>
    void DetachBlock(const T firstIndex, const T indexAfterLast, Func &&rBlockFunc, const std::size_t blockNum = 0 THREADPOOL_PRIORITY_INPUT) {
        if(indexAfterLast > firstIndex) {
            const Block blks(firstIndex, indexAfterLast, blockNum ? blockNum : m_threadNum);
            for(std::size_t blk = 0; blk < blks.GetBlockNum(); ++blk)
                DetachTask([blockFunc = std::forward<Func>(rBlockFunc), start = blks.start(blk), end = blks.end(blk)] {
                    blockFunc(start, end);
                } THREADPOOL_PRIORITY_OUTPUT);
        }
    }

    /**
     * @brief Parallelize a loop by automatically splitting it into blocks and submitting each block separately to the queue, with the specified priority. The loop function takes one argument, the loop index, so that it is called many times per block. Does not return a `MultiFuture`, so the user must use `Wait()` or some other method to ensure that the loop finishes executing, otherwise bad things will happen.
     *
     * @tparam T The type of the indices. Should be a signed or unsigned integer.
     * @tparam Func The type of the function to loop through.
     * @param firstIndex The first index in the loop.
     * @param indexAfterLast The index after the last index in the loop. The loop will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no blocks will be submitted.
     * @param rLoopFunc The function to loop through. Will be called once per index, many times per block. Should take exactly one argument: the loop index.
     * @param blockNum The maximum number of blocks to split the loop into. The default is 0, which means the number of blocks will be equal to the number of threads in the pool.
     * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `THREADPOOL_ENABLE_PRIORITY` is defined.
     */
    template<typename T, typename Func>
    void DetachLoop(const T firstIndex, const T indexAfterLast, Func &&rLoopFunc, const std::size_t blockNum = 0 THREADPOOL_PRIORITY_INPUT) {
        if(indexAfterLast > firstIndex) {
            const Block blks(firstIndex, indexAfterLast, blockNum ? blockNum : m_threadNum);
            for(std::size_t blk = 0; blk < blks.GetBlockNum(); ++blk) {
                DetachTask([loopFunc = std::forward<Func>(rLoopFunc), start = blks.start(blk), end = blks.end(blk)] {
                    for(T i = start; i < end; ++i) {
                        loopFunc(i);
                    }
                } THREADPOOL_PRIORITY_OUTPUT);
            }
        }
    }

    /**
     * @brief Submit a sequence of tasks enumerated by indices to the queue, with the specified priority. Does not return a `multi_future`, so the user must use `Wait()` or some other method to ensure that the sequence finishes executing, otherwise bad things will happen.
     *
     * @tparam T The type of the indices. Should be a signed or unsigned integer.
     * @tparam Func The type of the function used to define the sequence.
     * @param firstIndex The first index in the sequence.
     * @param indexAfterLast The index after the last index in the sequence. The sequence will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no tasks will be submitted.
     * @param rSequenceFunc The function used to define the sequence. Will be called once per index. Should take exactly one argument, the index.
     * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `THREADPOOL_ENABLE_PRIORITY` is defined.
     */
    template<typename T, typename Func>
    void DetachSequence(const T firstIndex, const T indexAfterLast, Func &&rSequenceFunc THREADPOOL_PRIORITY_INPUT) {
        for(T i = firstIndex; i < indexAfterLast; ++i)
            DetachTask([sequenceFunc = std::forward<Func>(rSequenceFunc), i] {
                sequenceFunc(i);
            } THREADPOOL_PRIORITY_OUTPUT);
    }

    /**
     * @brief Reset the pool with the total number of hardware threads available, as reported by the implementation. Waits for all currently running tasks to be completed, then destroys all threads in the pool and creates a new thread pool with the new number of threads. Any tasks that were waiting in the queue before the pool was Reset will then be executed by the new threads. If the pool was paused before resetting it, the new pool will be paused as well.
     */
    virtual void Reset();

    /**
     * @brief Reset the pool with a new number of threads. Waits for all currently running tasks to be completed, then destroys all threads in the pool and creates a new thread pool with the new number of threads. Any tasks that were waiting in the queue before the pool was Reset will then be executed by the new threads. If the pool was paused before resetting it, the new pool will be paused as well.
     *
     * @param threadsNum The number of threads to use.
     */
    virtual void Reset(const concurrency_t threadsNum);

    /**
     * @brief Reset the pool with the total number of hardware threads available, as reported by the implementation, and a new initialization function. Waits for all currently running tasks to be completed, then destroys all threads in the pool and creates a new thread pool with the new number of threads and initialization function. Any tasks that were waiting in the queue before the pool was Reset will then be executed by the new threads. If the pool was paused before resetting it, the new pool will be paused as well.
     *
     * @param threadsNum An initialization function to run in each thread before it starts to execute any submitted tasks. The function must take no arguments and have no return value. It will only be executed exactly once, when the thread is first constructed.
     */
    virtual void Reset(const std::function<void()> &rInitTaskFunc);

    /**
     * @brief Reset the pool with a new number of threads and a new initialization function. Waits for all currently running tasks to be completed, then destroys all threads in the pool and creates a new thread pool with the new number of threads and initialization function. Any tasks that were waiting in the queue before the pool was Reset will then be executed by the new threads. If the pool was paused before resetting it, the new pool will be paused as well.
     *
     * @param threadsNum The number of threads to use.
     * @param rInitTaskFunc An initialization function to run in each thread before it starts to execute any submitted tasks. The function must take no arguments and have no return value. It will only be executed exactly once, when the thread is first constructed.
     */
    virtual void Reset(const concurrency_t threadsNum, const std::function<void()> &rInitTaskFunc);

    /**
     * @brief Submit a function with no arguments into the task queue, with the specified priority. To submit a function with arguments, enclose it in a lambda expression. If the function has a return value, get a future for the eventual returned value. If the function has no return value, get an `std::future<void>` which can be used to Wait until the task finishes.
     *
     * @tparam Func The type of the function.
     * @tparam Result The return type of the function (can be `void`).
     * @param rTaskFunc The function to submit.
     * @param priority The priority of the task. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `THREADPOOL_ENABLE_PRIORITY` is defined.
     * @return A future to be used later to Wait for the function to finish executing and/or obtain its returned value if it has one.
     */
    template<typename Func, typename Result = std::invoke_result_t<std::decay_t<Func>>>
    [[nodiscard]] std::future<Result> SubmitTask(Func &&rTaskFunc THREADPOOL_PRIORITY_INPUT) {
        const std::shared_ptr<std::promise<Result>> pTaskPromise = std::make_shared<std::promise<Result>>();
        DetachTask([taskFunc = std::forward<Func>(rTaskFunc), pTaskPromise]() {
#ifndef THREADPOOL_DISABLE_EXCEPTION_HANDLING
            try {
#endif
                if constexpr(std::is_void_v<Result>) {
                    taskFunc();
                    pTaskPromise->set_value();
                } else {
                    pTaskPromise->set_value(taskFunc());
                }
#ifndef THREADPOOL_DISABLE_EXCEPTION_HANDLING
            } catch(...) {
                try {
                    pTaskPromise->set_exception(std::current_exception());
                } catch(...) {
                }
            }
#endif
        } THREADPOOL_PRIORITY_OUTPUT);

        return pTaskPromise->get_future();
    }

    /**
 * @brief Parallelize a loop by automatically splitting it into blocks and submitting each block separately to the queue, with the specified priority. The block function takes two arguments, the start and end of the block, so that it is only called only once per block, but it is up to the user make sure the block function correctly deals with all the indices in each block. Returns a `multi_future` that contains the futures for all of the blocks.
 *
 * @tparam T The type of the indices. Should be a signed or unsigned integer.
 * @tparam Func The type of the function to loop through.
 * @tparam Result The return type of the function to loop through (can be `void`).
 * @param firstIndex The first index in the loop.
 * @param indexAfterLast The index after the last index in the loop. The loop will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no blocks will be submitted, and an empty `multi_future` will be returned.
 * @param rBlockFunc A function that will be called once per block. Should take exactly two arguments: the first index in the block and the index after the last index in the block. `block(start, end)` should typically involve a loop of the form `for (T i = start; i < end; ++i)`.
 * @param blockNum The maximum number of blocks to split the loop into. The default is 0, which means the number of blocks will be equal to the number of threads in the pool.
 * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `THREADPOOL_ENABLE_PRIORITY` is defined.
 * @return A `MultiFuture` that can be used to Wait for all the blocks to finish. If the block function returns a value, the `multi_future` can also be used to obtain the values returned by each block.
 */
    template<typename T, typename Func, typename Result = std::invoke_result_t<std::decay_t<Func>, T, T>>
    [[nodiscard]] MultiFuture<Result> SubmitBlock(const T firstIndex, const T indexAfterLast, Func &&rBlockFunc, const std::size_t blockNum = 0 THREADPOOL_PRIORITY_INPUT) {
        if(indexAfterLast > firstIndex) {
            const Block blks(firstIndex, indexAfterLast, blockNum ? blockNum : m_threadNum);
            MultiFuture<Result> future;
            future.reserve(blks.GetBlockNum());
            for(size_t blk = 0; blk < blks.GetBlockNum(); ++blk)
                future.push_back(SubmitTask([blockFunc = std::forward<Func>(rBlockFunc), start = blks.start(blk), end = blks.end(blk)] {
                    return blockFunc(start, end);
                } THREADPOOL_PRIORITY_OUTPUT));
            return future;
        }
        return {};
    }

    /**
 * @brief Parallelize a loop by automatically splitting it into blocks and submitting each block separately to the queue, with the specified priority. The loop function takes one argument, the loop index, so that it is called many times per block. It must have no return value. Returns a `multi_future` that contains the futures for all of the blocks.
 *
 * @tparam T The type of the indices. Should be a signed or unsigned integer.
 * @tparam Func The type of the function to loop through.
 * @param firstIndex The first index in the loop.
 * @param indexAfterLast The index after the last index in the loop. The loop will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no tasks will be submitted, and an empty `multi_future` will be returned.
 * @param rLoopFunc The function to loop through. Will be called once per index, many times per block. Should take exactly one argument: the loop index. It cannot have a return value.
 * @param blockNum The maximum number of blocks to split the loop into. The default is 0, which means the number of blocks will be equal to the number of threads in the pool.
 * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `THREADPOOL_ENABLE_PRIORITY` is defined.
 * @return A `multi_future` that can be used to Wait for all the blocks to finish.
 */
    template<typename T, typename Func>
    [[nodiscard]] MultiFuture<void> SubmitLoop(const T firstIndex, const T indexAfterLast, Func &&rLoopFunc, const std::size_t blockNum = 0 THREADPOOL_PRIORITY_INPUT) {
        if(indexAfterLast > firstIndex) {
            const Block blks(firstIndex, indexAfterLast, blockNum ? blockNum : m_threadNum);
            MultiFuture<void> future;
            future.reserve(blks.GetBlockNum());
            for(std::size_t blk = 0; blk < blks.GetBlockNum(); ++blk)
                future.push_back(SubmitTask([loopFunc = std::forward<Func>(rLoopFunc), start = blks.start(blk), end = blks.end(blk)] {
                    for(T i = start; i < end; ++i) {
                        loopFunc(i);
                    }
                } THREADPOOL_PRIORITY_OUTPUT));
            return future;
        }
        return {};
    }

    /**
 * @brief Submit a sequence of tasks enumerated by indices to the queue, with the specified priority. Returns a `multi_future` that contains the futures for all of the tasks.
 *
 * @tparam T The type of the indices. Should be a signed or unsigned integer.
 * @tparam Func The type of the function used to define the sequence.
 * @tparam Result The return type of the function used to define the sequence (can be `void`).
 * @param firstIndex The first index in the sequence.
 * @param indexAfterLast The index after the last index in the sequence. The sequence will iterate from `first_index` to `(index_after_last - 1)` inclusive. In other words, it will be equivalent to `for (T i = first_index; i < index_after_last; ++i)`. Note that if `index_after_last <= first_index`, no tasks will be submitted, and an empty `multi_future` will be returned.
 * @param rSequenceFunc The function used to define the sequence. Will be called once per index. Should take exactly one argument, the index.
 * @param priority The priority of the tasks. Should be between -32,768 and 32,767 (a signed 16-bit integer). The default is 0. Only enabled if `THREADPOOL_ENABLE_PRIORITY` is defined.
 * @return A `multi_future` that can be used to Wait for all the tasks to finish. If the sequence function returns a value, the `multi_future` can also be used to obtain the values returned by each task.
 */
    template<typename T, typename Func, typename Result = std::invoke_result_t<std::decay_t<Func>, T>>
    [[nodiscard]] MultiFuture<Result> SubmitSequence(const T firstIndex, const T indexAfterLast, Func &&rSequenceFunc THREADPOOL_PRIORITY_INPUT) {
        if(indexAfterLast > firstIndex) {
            MultiFuture<Result> future;
            future.reserve(static_cast<size_t>(indexAfterLast - firstIndex));
            for(T i = firstIndex; i < indexAfterLast; ++i)
                future.push_back(SubmitTask([sequenceFunc = std::forward<Func>(rSequenceFunc), i] {
                    return sequenceFunc(i);
                } THREADPOOL_PRIORITY_OUTPUT));
            return future;
        }
        return {};
    }

    /**
 * @brief Unpause the pool. The workers will resume retrieving new tasks out of the queue. Only enabled if `THREADPOOL_ENABLE_PAUSE` is defined.
 */
    virtual void Resume();

    // Macros used internally to enable or disable pausing in the waiting and worker functions.
    inline bool ThreadPoolPausedOrEmpty() {
#ifdef THREADPOOL_ENABLE_PAUSE
        return (m_isPaused || m_taskQueue.empty());
#else
        return m_taskQueue.empty();
#endif
    }

    /**
     * @brief Wait for tasks to be completed. Normally, this function waits for all tasks, both those that are currently running in the threads and those that are still waiting in the queue. However, if the pool is paused, this function only waits for the currently running tasks (otherwise it would Wait forever). Note: To wait for just one specific task, use `SubmitTask()` instead, and call the `Wait()` member function of the generated future.
     *
     * @throws `WaitDeadlock` if called from within a thread of the same pool, which would result in a deadlock. Only enabled if `THREADPOOL_ENABLE_WAIT_DEADLOCK_CHECK` is defined.
     */
    virtual void Wait();

    /**
     * @brief Wait for tasks to be completed, but stop waiting after the specified duration has passed.
     *
     * @tparam R An arithmetic type representing the number of ticks to Wait.
     * @tparam P An `std::ratio` representing the length of each tick in seconds.
     * @param rDuration The amount of time to Wait.
     * @return `true` if all tasks finished running, `false` if the duration expired but some tasks are still running.
     *
     * @throws `WaitDeadlock` if called from within a thread of the same pool, which would result in a deadlock. Only enabled if `THREADPOOL_ENABLE_WAIT_DEADLOCK_CHECK` is defined.
     */
    template<typename R, typename P>
    bool WaitFor(const std::chrono::duration<R, P> &rDuration) {
#ifdef THREADPOOL_ENABLE_WAIT_DEADLOCK_CHECK
        if(this_thread::get_pool() == this)
            throw WaitDeadlock();
#endif
        std::unique_lock lock(m_taskMutex);
        m_isWaiting = true;
        const bool status = m_taskDoneCV.wait_for(lock, rDuration, [this] {
            return (m_runningTaskNum == 0) && ThreadPoolPausedOrEmpty();
        });
        m_isWaiting = false;
        return status;
    }

    /**
     * @brief Wait for tasks to be completed, but stop waiting after the specified time point has been reached.
     *
     * @tparam C The type of the clock used to measure time.
     * @tparam D An `std::chrono::duration` type used to indicate the time point.
     * @param rTimeoutTime The time point at which to stop waiting.
     * @return `true` if all tasks finished running, `false` if the time point was reached but some tasks are still running.
     *
     * @throws `WaitDeadlock` if called from within a thread of the same pool, which would result in a deadlock. Only enabled if `THREADPOOL_ENABLE_WAIT_DEADLOCK_CHECK` is defined.
     */
    template<typename C, typename D>
    bool WaitUntil(const std::chrono::time_point<C, D> &rTimeoutTime) {
#ifdef THREADPOOL_ENABLE_WAIT_DEADLOCK_CHECK
        if(this_thread::get_pool() == this)
            throw WaitDeadlock();
#endif
        std::unique_lock lock(m_taskMutex);
        m_isWaiting = true;
        const bool status = m_taskDoneCV.wait_until(lock, rTimeoutTime, [this] {
            return (m_runningTaskNum == 0) && ThreadPoolPausedOrEmpty();
        });
        m_isWaiting = false;

        return status;
    }

    /**
     * @brief An exception that will be thrown by `Wait()`, `WaitFor()`, and `WaitUntil()` if the user tries to call them from within a thread of the same pool, which would result in a deadlock.
     */
    struct WaitDeadlock: public std::runtime_error {
        WaitDeadlock(): std::runtime_error("common::ThreadPool::WaitDeadlock") {};
    };

 private:
    /**
     * @brief Determine how many threads the pool should have, based on the parameter passed to the constructor or Reset().
     *
     * @param threadsNum The parameter passed to the constructor or `Reset()`. If the parameter is a positive number, then the pool will be created with this number of threads. If the parameter is non-positive, or a parameter was not supplied (in which case it will have the default value of 0), then the pool will be created with the total number of hardware threads available, as obtained from `std::thread::hardware_concurrency()`. If the latter returns zero for some reason, then the pool will be created with just one thread.
     * @return The number of threads to use for constructing the pool.
     */
    [[nodiscard]] static concurrency_t DetermineThreadNum(const concurrency_t threadsNum);

    /**
     * @brief A worker function to be assigned to each thread in the pool. Waits until it is notified by `DetachTask()` that a task is available, and then retrieves the task from the queue and executes it. Once the task finishes, the Run notifies `Wait()` in case it is waiting.
     *
     * @param idx The index of this thread.
     * @param rInitTaskFunc An initialization function to run in this thread before it starts to execute any submitted tasks.
     */
    void Run(const concurrency_t idx, const std::function<void()> &rInitTaskFunc);

    /**
     * @brief A helper class to divide a range into blocks. Used by `DetachBlock()`, `SubmitBlock()`, `DetachLoop()`, and `SubmitLoop()`.
     *
     * @tparam T The type of the indices. Should be a signed or unsigned integer.
     */
    template<typename T>
    class [[nodiscard]] Block {
     public:
        /**
         * @brief Construct a `blocks` object with the given specifications.
         *
         * @param firstIndex The first index in the range.
         * @param indexAfterLast The index after the last index in the range.
         * @param blockNum The desired number of blocks to divide the range into.
         */
        Block(const T firstIndex, const T indexAfterLast, const size_t blockNum): m_firstIndex(firstIndex), m_indexAfterLast(indexAfterLast), m_blockNum(blockNum) {
            if(m_indexAfterLast > m_firstIndex) {
                const std::size_t totalSize = static_cast<size_t>(m_indexAfterLast - m_firstIndex);
                if(m_blockNum > totalSize)
                    m_blockNum = totalSize;
                m_blockSize = totalSize / m_blockNum;
                m_remainder = totalSize % m_blockNum;
                if(m_blockSize == 0) {
                    m_blockSize = 1;
                    m_blockNum = (totalSize > 1) ? totalSize : 1;
                }
            } else {
                m_blockNum = 0;
            }
        }

        /**
         * @brief Get the first index of a block.
         *
         * @param block The block number.
         * @return The first index.
         */
        [[nodiscard]] T start(const std::size_t block) const {
            return m_firstIndex + static_cast<T>(block * m_blockSize) + static_cast<T>(block < m_remainder ? block : m_remainder);
        }

        /**
         * @brief Get the index after the last index of a block.
         *
         * @param block The block number.
         * @return The index after the last index.
         */
        [[nodiscard]] T end(const std::size_t block) const {
            return (block == m_blockNum - 1) ? m_indexAfterLast : start(block + 1);
        }

        /**
         * @brief Get the number of blocks. Note that this may be different than the desired number of blocks that was passed to the constructor.
         *
         * @return The number of blocks.
         */
        [[nodiscard]] std::size_t GetBlockNum() const {
            return m_blockNum;
        }

     private:
        /**
         * @brief The size of each block (except possibly the last block).
         */
        size_t m_blockSize{0};

        /**
         * @brief The first index in the range.
         */
        T m_firstIndex{0};

        /**
         * @brief The index after the last index in the range.
         */
        T m_indexAfterLast{0};

        /**
         * @brief The number of blocks.
         */
        size_t m_blockNum{0};

        /**
         * @brief The remainder obtained after dividing the total size by the number of blocks.
         */
        size_t m_remainder{0};
    };// class blocks

#ifdef THREADPOOL_ENABLE_PRIORITY
    /**
     * @brief A helper class to store a task with an assigned priority.
     */
    class [[nodiscard]] pr_task {
        friend class ThreadPool;

     public:
        /**
         * @brief Construct a new task with an assigned priority by copying the task.
         *
         * @param task_ The task.
         * @param priority_ The desired priority.
         */
        explicit pr_task(const std::function<void()> &task_, const priority_t priority_ = 0): task(task_), priority(priority_) {}

        /**
         * @brief Construct a new task with an assigned priority by moving the task.
         *
         * @param task_ The task.
         * @param priority_ The desired priority.
         */
        explicit pr_task(std::function<void()> &&task_, const priority_t priority_ = 0): task(std::move(task_)), priority(priority_) {}

        /**
         * @brief Compare the priority of two tasks.
         *
         * @param lhs The first task.
         * @param rhs The second task.
         * @return `true` if the first task has a lower priority than the second task, `false` otherwise.
         */
        [[nodiscard]] friend bool operator<(const pr_task &lhs, const pr_task &rhs) {
            return lhs.priority < rhs.priority;
        }

     private:
        /**
         * @brief The task.
         */
        std::function<void()> task = {};

        /**
         * @brief The priority of the task.
         */
        priority_t priority = 0;
    };// class pr_task
#endif

    /**
     * @brief A flag indicating whether the workers should pause. When set to `true`, the workers temporarily stop retrieving new tasks out of the queue, although any tasks already executed will keep running until they are finished. When set to `false` again, the workers resume retrieving tasks.
     */
    bool m_isPaused = false;

    /**
     * @brief A condition variable to notify `Run()` that a new task has become available.
     */
    std::condition_variable m_taskAvailableCV{};

    /**
     * @brief A condition variable to notify `Wait()` that the tasks are done.
     */
    std::condition_variable m_taskDoneCV{};

    /**
     * @brief A queue of tasks to be executed by the threads.
     */
#ifdef THREADPOOL_ENABLE_PRIORITY
    std::priority_queue<pr_task> m_taskQueue = {};
#else
    std::queue<std::function<void()>> m_taskQueue{};
#endif

    /**
     * @brief A counter for the total number of currently running tasks.
     */
    size_t m_runningTaskNum{0};

    /**
     * @brief A mutex to synchronize access to the task queue by different threads.
     */
    mutable std::mutex m_taskMutex{};

    /**
     * @brief The number of threads in the pool.
     */
    concurrency_t m_threadNum{0};

    /**
     * @brief A smart pointer to manage the memory allocated for the threads.
     */
    std::unique_ptr<std::thread[]> m_pThreads{nullptr};

    /**
     * @brief A flag indicating that `Wait()` is active and expects to be notified whenever a task is done.
     */
    bool m_isWaiting{false};

    /**
     * @brief A flag indicating to the workers to keep running. When set to `false`, the workers terminate permanently.
     */
    bool m_isRunning{false};
};// class ThreadPool
}// namespace common

// module common.threadpool;
// module;