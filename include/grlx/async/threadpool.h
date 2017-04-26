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
        : running(true)
    {
        for(auto i = 0; i < threadCount; i++)
        {
            workers.emplace_back( [this]()
            {
                for(;;)
                {

                    TaskType task;
                    {
                        std::unique_lock<std::mutex> lock(sync);
                        cv.wait(lock, [this]() ->bool { return !this->running || !this->scheduler.empty(); });
                        if(!running && scheduler.empty()){
                            return;
                        }
                        task = std::move( scheduler.top() );
                        scheduler.pop();
                    }
                    task();
                }

            });

        }

    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(sync);
            running = false;
            scheduler.clear();
        }


        cv.notify_all();
        for(auto& w: workers)
        {
            w.join();
        }
    }

    template<typename F, typename ...Args>
    auto schedule( F&& func, Args&&... args) -> std::future<decltype(func(args...))>
    {
        std::unique_lock<std::mutex> lock(sync);

        using PT = std::packaged_task<decltype(func(args...))()>;

        std::shared_ptr<PT> task(new PT(std::bind( std::forward<F>(func), std::forward<Args>(args)...) ));

        auto future = task->get_future();

        std::function<void()> work = std::bind([task](){

            (*task)();
        });

        scheduler.push( std::move(work) );

        cv.notify_one();

        return future;
    }

private:




private:
    std::mutex sync;
    std::condition_variable cv;
    bool running;
    SchedulingPolicy<TaskType> scheduler;
    std::vector<std::thread> workers;
};


using FifoThreadPool = ThreadPool< FifoScheduler >;

}
}
