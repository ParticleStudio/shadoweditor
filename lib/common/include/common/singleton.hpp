#ifndef SHADOW_SINGLETON_H
#define SHADOW_SINGLETON_H

#include <cassert>
#include <cstdlib>
#include <mutex>

#include "common/common.h"

namespace shadow::singleton {
/////////////////////////////////////////////////
/**
 * @file singleton.cppm
 * @brief  单例类 .
 *
 * 单例实现类
 *
 * 没有实现对单例生命周期的管理,使用示例代码如下:
 *
 * class A : public Singleton<A, CreateStatic,  DefaultLifetime>
 *
 * {
 *
 *  public:
 *
 *    A(){cout << "A" << endl;}
 *
 *   ~A() {
 *      cout << "~A" << endl;
 *    }
 *
 *    void test(){cout << "test A" << endl;}
 *
 * };
 *
 * 对象的创建方式由CreatePolicy指定, 有如下方式:
 *
 * CreateUsingNew: 在堆中采用new创建
 *
 * CreateStatic`: 在栈中采用static创建
 *
 * 对象生命周期管理由LifetimePolicy指定, 有如下方式:
 *
 * DefaultLifetime:缺省声明周期管理
 *
 *如果单例对象已经析够, 但是还有调用, 会触发异常
 *
 * PhoneixLifetime:不死生命周期
 *
 * 如果单例对象已经析够, 但是还有调用, 会再创建一个
 *
 * NoDestroyLifetime:不析够
 *
 * 对象创建后不会调用析够函数析够, 通常采用实例中的方式就可以了
 *
 */
/////////////////////////////////////////////////

/**
 * @brief 定义CreatePolicy:定义对象创建策略
 */
template<typename T>
class CreateUsingNew {
 public:
    /**
     * @brief  创建.
     *
     * @return T*
     */
    static T *Create() {
        return new T;
    }

    /**
	 * @brief 释放.
	 *
     * @param t
     */
    static void Destroy(T *t) {
        delete t;
    }
};

template<typename T>
class CreateStatic {
 public:
    /**
     * @brief   最大的空间
     */
    union MaxAlign {
        char m_t[sizeof(T)];
        long double m_longDouble;
    };

    /**
     * @brief   创建.
     *
     * @return T*
     */
    static T *Create() {
        static MaxAlign t;
        return new(&t) T;
    }

    /**
     * @brief   释放.
     *
     * @param t
     */
    static void Destroy(T *t) {
        t->~T();
    }
};

template<typename T>
class CreateRealStatic {
 public:
    /**
	 * @brief   创建.
     *
     * @return T*
     */
    static T *Create() {
        static T t;
        return &t;
    }

    /**
	 * @brief   释放.
	 *
     * @param t
     */
    static void Destroy(T *t) {
    }
};

////////////////////////////////////////////////////////////////
/**
 * @brief 定义LifetimePolicy:定义对象的声明周期管理
 * 进程退出时销毁对象
 */
template<typename T>
class DefaultLifetime {
 public:
    static void DeadReference() {
        throw std::logic_error("singleton object has dead.");
    }

    static void ScheduleDestruction(T *, void (*pFunc)()) {
        std::atexit(pFunc);
    }
};

/**
 * @brief 对象被销毁后可以重生(比如log,全局任何时候都需要)
 */
template<typename T>
class PhoneixLifetime {
 public:
    static void DeadReference() {
        m_bDestroyedOnce = true;
    }

    static void ScheduleDestruction(T *, void (*pFun)()) {
        if(!m_bDestroyedOnce) {
            std::atexit(pFun);
        }
    }

 private:
    static bool m_bDestroyedOnce;
};

template<class T>
bool PhoneixLifetime<T>::m_bDestroyedOnce = false;

/**
 * @brief 不做对象销毁
 */
template<typename T>
struct NoDestroyLifetime {
    static void ScheduleDestruction(T *, void (*)()) {
    }

    static void DeadReference() {
    }
};

//////////////////////////////////////////////////////////////////////
// Singleton
template<typename T, template<typename> class CreatePolicy = CreateUsingNew, template<typename> class LifetimePolicy = DefaultLifetime>
class COMMON_API Singleton {
 public:
    virtual ~Singleton() noexcept = default;

 public:
    /**
     * @brief 获取实例
     *
     * @return T*
     */
    static inline T *GetInstance() {
        static std::mutex sSingletonMutex;

        auto pInstance = m_pInstance.load();
        if(pInstance == nullptr) {
            std::scoped_lock<std::mutex> const lock(sSingletonMutex);
            pInstance = m_pInstance.load();
            if(pInstance == nullptr) {
                if(m_bDestroyed) {
                    LifetimePolicy<T>::DeadReference();
                    m_bDestroyed = false;
                }

                pInstance = CreatePolicy<T>::Create();
                m_pInstance.store(pInstance);
                LifetimePolicy<T>::ScheduleDestruction(m_pInstance, &destroy);
            }
        }

        return pInstance;
    }

 protected:
    Singleton() = default;
    Singleton(const Singleton &) = default;
    Singleton &operator=(const Singleton &) = default;

 protected:
    static void destroy() {
        assert(!m_bDestroyed);
        CreatePolicy<T>::Destroy(static_cast<T *>(m_pInstance));
        m_pInstance = nullptr;
        m_bDestroyed = true;
    }

 protected:
    static std::atomic<T *> m_pInstance;
    static bool m_bDestroyed;
};

template<class T, template<class> class CreatePolicy, template<class> class LifetimePolicy>
std::atomic<T *> Singleton<T, CreatePolicy, LifetimePolicy>::m_pInstance{nullptr};

template<class T, template<class> class CreatePolicy, template<class> class LifetimePolicy>
bool Singleton<T, CreatePolicy, LifetimePolicy>::m_bDestroyed{false};
} // namespace shadow::singleton

#endif // SHADOW_SINGLETON_H
