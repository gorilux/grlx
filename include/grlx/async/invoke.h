#pragma once

////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2015 David Salvador Pinheiro
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is David Salvador Pinheiro
///
/// @author David Salvador Pinheiro
/// @author Copyright 2015, David Salvador Pinheiro
////////////////////////////////////////////////////////////////////////////////


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
