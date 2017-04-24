#pragma once

#include <thread>
#include <future>

namespace grlx
{

template<typename F, typename ...Args>
auto async( F&& f, Args&&... args) -> std::future<decltype(f(args...))>
{
    std::packaged_task<decltype(f(args...))>
            task(std::bind( std::forward(f), std::forward<Args>...));

    auto future = task.get_future();

    std::thread thread( std::move( task ) );
    thread.detach();

    return future;
}

}
