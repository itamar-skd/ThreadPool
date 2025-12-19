#include "threadpool.h"

ThreadPool::ThreadPool()
    : __keep_alive(true)
{
    for (size_t i = 0; i < NUM_THREADS; i++)
        this->__threads[i] = std::thread(&ThreadPool::__work, this);
}

ThreadPool::~ThreadPool()
{
    this->__keep_alive = false;
    this->__cv.notify_all();
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        if (this->__threads[i].joinable())
            this->__threads[i].join();
    }
}

ThreadPool& ThreadPool::pool()
{
    static ThreadPool pool_;
    return pool_;
}

void ThreadPool::__work()
{
    while (this->__keep_alive.load())
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lk_(this->__tasks_sync);
            this->__cv.wait(lk_, [this] { return (!this->__tasks.empty() || !this->__keep_alive); });

            if (!__keep_alive && this->__tasks.empty())
                return;

            task = std::move(this->__tasks.front());
            this->__tasks.pop();
        }

        task();
    }
}