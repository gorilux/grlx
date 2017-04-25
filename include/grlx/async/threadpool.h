#pragma once

#include <thread>
#include <future>
#include <functional>
#include <deque>
#include <mutex>
#include <condition_variable>

namespace grlx
{
namespace async
{

using TaskFunc = std::function<void()>;

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



template<
    template <typename> class SchedulingPolicy = FifoScheduler
>
class ThreadPool
{
public:

    using TaskType = TaskFunc;

    ThreadPool( std::size_t threadCount = std::thread::hardware_concurrency() )
    {

    }

    template<typename F, typename ...Args>
    auto schedule( F&& func, Args&&... args) -> std::future<decltype(func(args...))>
    {
        std::packaged_task<decltype(f(args...))()>
                task(std::bind( std::forward<F>(func), std::forward<Args>(args)...));

        auto future = task.get_future();

        TaskType work = [t = std::move(task)](){
            t();
        };

        scheduler.push( std::move(work) );

        return future;
    }

private:


    SchedulingPolicy<TaskType> scheduler;

private:

    std::vector<std::thread> workers;
};


using FifoThreadPool = ThreadPool< FifoScheduler >;

}
}
