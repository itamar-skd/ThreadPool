#ifndef __THREADPOOL_SRC_H__
#define __THREADPOOL_SRC_H__

#include <stddef.h>
#include <cstdint>
#include <queue>
#include <thread>
#include <functional>
#include <future>
#include <mutex>
#include <memory.h>
#include <condition_variable>
#include <atomic>

#define NUM_THREADS 4

class ThreadPool final
{
    // Singleton pattern: private constructor and deleted copy/move operations
    private:
        ThreadPool();
        ~ThreadPool();
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;
    public:
        static ThreadPool& pool();

    public:
        template<typename F>
        auto submit(F&& func) -> std::future<std::invoke_result_t<std::decay_t<F>>>
        {
            using R = std::invoke_result_t<std::decay_t<F>>;

            /**
             * make a task out of the passed function.
             * 
             * packaged tasks essentially wrap any callable function to execute it asynchronously.
             * they return an std::future, which is an object that when called "get" upon waits for the task to be executed then returns the value returned from the function.
             * 
             * std::forward preserves the leftness/rightness of a function. without it, f is always an lvalue, which means that it would always be copied, resulting in expensive copies or information loss.
             * r-value functions are moved, l-value functions are copied.
             */
            std::shared_ptr<std::packaged_task<R()>> task = std::make_shared<std::packaged_task<R()>>(std::forward<F>(func));
            std::future<R> f = task->get_future();

            /**
             * packaged tasks are dependent on their return types. we can't make a queue out of objects who are type-dependent.
             * once we have the future object, the threads don't really care about the return value of the tasks they're handling. they just execute them.
             * as a result, we can disregard the return value and wrap those tasks in a void lambda function that executes the task.
             */
            auto wrapper = [task]() mutable {
                (*task)();
            };

            {
                std::lock_guard<std::mutex> lk(this->__tasks_sync);
                this->__tasks.push(std::move(wrapper));
            }
            
            this->__cv.notify_one();
            return f;
        }

        void __work();

    private: /* pool */
        std::thread __threads[NUM_THREADS];
        std::mutex __tasks_sync;
        std::queue<std::function<void()>> __tasks;    
        std::atomic<bool> __keep_alive;
        std::condition_variable __cv;
};

#endif /* __THREADPOOL_SRC_H__ */