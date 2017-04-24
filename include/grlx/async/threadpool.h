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

    const TaskType& top() const
    {
        return container.front();
    }

    TaskType& top()
    {
        return container.front();
    }
    std::size_t size() const
    {
        return container.size();
    }
    bool empty() const
    {
        return container.empty();
    }
    void clear()
    {
        container.clear();
    }

protected:
    std::deque<TaskType> container;

};



//template< template <typename> class SchedulingPolicy =
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


    //FifoScheduler<> scheduler;

private:

    std::vector<std::thread> workers;
};


}
}
