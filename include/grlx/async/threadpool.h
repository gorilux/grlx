#pragma once

#include <thread>
#include <future>
#include <functional>
#include <deque>

namespace grlx
{
namespace async
{

template<typename Task>
class FifoScheduler
{
public:

    using TaskType = Task;
    bool push( TaskType&& task)
    {
        container.push_back( std::forward<TaskType>(task));
        return true;
    }
    void pop()
    {
        container.pop_front();
    }

    TaskType&& top() const
    {
        return container.front();
    }

protected:
    std::deque<TaskType> container;

};




class ThreadPool
{
public:
    ThreadPool( std::size_t threadCount = std::thread::hardware_concurrency() )
    {

    }

    template<typename F, typename ...Args>
    auto schedule( F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {

    }

private:



private:

    std::vector<std::thread> workers;
};


}
}
