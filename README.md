# Thread Pool
## Introduction
This is small project I made while trying to learn advanced thread management. Because of this, this project is full of documentation (especially for topics I was not familiar with prior to this project) for both myself and others to learn in the future. Some of this documentation is presented below.

## What is a Thread Pool?
Creating a thread is quite expensive. The creation of a thread requires OS allocation, stack creation and scheduling overhead. While threads are great for asynchronous tasks, their creation can be quite heavy when they're used for many short, simple tasks.<br/>
Thread pools are meant to solve that. They are a collection of threads that are always waiting to be assigned a task. They are created typically once when the program starts, then joined/destroyed only when the program quits (or in some rare cases, by request).<br/>
When a task needs to be completed asynchronously, it's simply inserted into a queue then waits for its turn. Obviously, how fast a task gets done depends on how many threads are available to the pool, because the number of threads determine how many tasks can be done at once.<br/>
For a more detailed overview of the algorithm, please see the next section.

## Usage Example
```cpp
auto future = ThreadPool::pool().submit([] {
    return 42;
});

std::cout << future.get() << std::endl;
```

## How does a Thread Pool work?
### Thread Pool
1. A thread pool class is generated and `NUM_THREADS` threads are created, they each run on a while loop that stays for as long as `__keep_alive` is true.
2. When a new task is submitted, the pool wraps it in an `std::packaged_task`. Packaged tasks are essentially tasks that should be done asynchronously. These objects return an `std::future` object. Per the type's name, this is an object that **will** hold the value returned from the task when it is finished. When its `.get()` method is called, the thread the method was called from waits for the task to finish, then returns the value returned from the task.
3. When creating the packaged task, the function passed to the `submit` function is moved to the task using `std::forward`. This is a method that conditionally casts an object as an r-value only if it was created as an r-value. An r-value object will be moved, while an l-value object will be copied. This is meant to refrain heavy copies or data loss.
4. Packaged tasks are type-dependent. They depend on the return type. Our `__tasks` queue, however, is not and should hold tasks with any return type. To handle this, we must realize that once we have the `std::future` object held by the packaged task, we don't care about the returned value. The only one who cares about it is the thread that submitted that task. Because of this, we can wrap the task in a lambda function that executes the task without saving its returned value. The thread that holds the `std::future`, which we return at the end of the `submit` function, will take that value when it's available.
5. The `__tasks_sync` mutex is locked, then the lambda wrapper is pushed into the `__tasks` queue, to be handled by the threads.
6. The `std::future` object is returned to the thread that submitted the task.

### Threads
1. Each of the threads wait on a condition variable, `__cv`, only ever signaled by the `submit` function and the class's destructor.
2. When signaled, the first thread to pick up on the signal locks the `__tasks_sync` mutex, takes the task at the front of the `__tasks` queue, then unlocks the mutex.
3. The thread then performs the task, before going back to polling on `__cv`.

## Design Notes
1. This thread pool uses a FIFO queue.
2. The number of threads is fixed at compile time.
3. Tasks are executed in submission order, but completion order is not guaranteed (task #2 may be completed before task #1, despite being submitted before #1).
