#pragma once
xo
#include <thread>
#include <future>
#include <functional>

namespace grlx
{

namespace async
{

template<typename F, typename ...Args>
auto invoke( F&& f, Args&&... args) -> std::future<decltype(f(args...))>
{
    std::packaged_task<decltype(f(args...))()>
            task(std::bind( std::forward<F>(f), std::forward<Args>(args)...));

    auto future = task.get_future();

    std::thread thread( std::move( task ) );
    thread.detach();

    return future;
}

}
}
