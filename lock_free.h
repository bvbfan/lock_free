
#pragma once

#ifdef __clang__
#define LOCKABLE __attribute__((lockable))
#define SCOPED_LOCKABLE __attribute__((scoped_lockable))
#define UNLOCK_FUNCTION(...) __attribute__((unlock_function(__VA_ARGS__)))
#define EXCLUSIVE_LOCK_FUNCTION(...) __attribute__((exclusive_lock_function(__VA_ARGS__)))
#define EXCLUSIVE_TRYLOCK_FUNCTION(...) __attribute__((exclusive_trylock_function(__VA_ARGS__)))
#else
#define LOCKABLE
#define SCOPED_LOCKABLE
#define UNLOCK_FUNCTION(...)
#define EXCLUSIVE_LOCK_FUNCTION(...)
#define EXCLUSIVE_TRYLOCK_FUNCTION(...)
#endif

#include <atomic>
#include <thread>

class LOCKABLE CLockFreeMutex
{
    std::atomic_bool lf_lock{false};

public:
    bool try_lock() EXCLUSIVE_TRYLOCK_FUNCTION(true)
    {
        bool locked = false;
        return lf_lock.compare_exchange_weak(locked, true,
                                             std::memory_order_release,
                                             std::memory_order_relaxed);
    }
    void lock() EXCLUSIVE_LOCK_FUNCTION()
    {
        while (!try_lock());
    }
    void unlock() UNLOCK_FUNCTION()
    {
        lf_lock.store(false, std::memory_order_release);
    }
};

class LOCKABLE CLockFreeRecursiveMutex
{
    std::atomic_size_t owner_id{0};
    std::atomic_bool lf_lock{false};
    std::atomic_size_t locks_count{0};

    static size_t get_thread_id()
    {
        const thread_local auto thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
        return thread_id;
    }

public:
    bool try_lock() EXCLUSIVE_TRYLOCK_FUNCTION(true)
    {
        if (get_thread_id() == owner_id) {
            ++locks_count;
            return true;
        }

        bool locked = false;
        if (lf_lock.compare_exchange_weak(locked, true,
                                          std::memory_order_release,
                                          std::memory_order_relaxed))
        {
            locks_count = 1;
            owner_id = get_thread_id();
            return true;
        }
        return false;
    }
    void lock() EXCLUSIVE_LOCK_FUNCTION()
    {
        while (!try_lock());
    }
    void unlock() UNLOCK_FUNCTION()
    {
        if (locks_count.fetch_sub(1, std::memory_order_release) == 1) {
            owner_id = 0;
            lf_lock.store(false, std::memory_order_release);
        }
    }
};
