#include "threadpool.h"
#include <iostream>

double add(double a, double b) { return a + b; }

int main(int argc, char** argv)
{
    std::future<int>    first   = ThreadPool::pool().submit([]{ return 42; });
    std::future<double> second  = ThreadPool::pool().submit(add, 1, 3);

    std::cout << "first   " << first.get() << "\n";
    std::cout << "second  " << second.get() << "\n";
    return 0;
}