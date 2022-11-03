
#include <cassert>
#include <mutex>
#include <iostream>
#include <vector>

#include "lock_free.h"

template <typename T>
void lock_free_test(const int num_threads)
{
    std::atomic_int threads_count(num_threads);

    auto testFunc = [&threads_count]() {
        static T cs_lock;
        static std::atomic_int context(0);

        --threads_count; // every thread decrements count

        std::lock_guard lock(cs_lock);
        ++context;
        while (threads_count > 0); // wait all threads to enter
        assert(context.load() == 1); // but only one operates
        --context;
    };

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; i++)
        threads.emplace_back(testFunc);

    for (auto& thread : threads)
        thread.join();
}

void try_lock_free_mutex()
{
    CLockFreeMutex cs_lock;
    {
        // hold the lock
        std::lock_guard lock(cs_lock);
        // lock is non-recursive
        assert(!cs_lock.try_lock());
    }
    // lock is released
    assert(cs_lock.try_lock());
}

void try_lock_free_recursive_mutex()
{
    CLockFreeRecursiveMutex cs_lock;
    {
        // hold the lock
        std::lock_guard lock(cs_lock);
        // lock is recursive
        assert(cs_lock.try_lock());
    }
    // lock is released
    assert(cs_lock.try_lock());
}

int main()
{
    std::cout << "Test lock free mutex\n";
    lock_free_test<CLockFreeMutex>(256);

    std::cout << "Test lock free recursive mutex\n";
    lock_free_test<CLockFreeRecursiveMutex>(256);

    std::cout << "Test try lock free mutex\n";
    try_lock_free_mutex();

    std::cout << "Test try lock free recursive mutex\n";
    try_lock_free_recursive_mutex();

    std::cout << "Test suite ends successfully\n";
    return 0;
}
