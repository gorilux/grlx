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
#ifndef GRLX_ASYNC_QUEUE_H
#define GRLX_ASYNC_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

namespace grlx
{
namespace async
{

template<typename T>
class BlockingQueue
{
public:
    BlockingQueue():
        disposing(false)
    {

    }
    ~BlockingQueue()
    {
        disposing = true;
        dataCond.notify_all();
    }
    void pushBack(const T & value)
    {
        std::unique_lock<std::mutex> lock(sync);
        dataQueue.push_back(value);
        dataCond.notify_one();
    }

    void pushFront(const T & value)
    {

       std::unique_lock<std::mutex> lock(sync);
        dataQueue.push_front(value);
        dataCond.notify_one();
    }

    bool popFront(T& value)
    {
       std::unique_lock<std::mutex> lock(sync);

        if(dataQueue.empty())
            dataCond.wait(lock);

        if(disposing)
            return false;

        value = dataQueue.front();
        dataQueue.pop_front();
        return true;
    }

    bool popBack(T& value)
    {
        std::unique_lock<std::mutex> lock(sync);

        if(dataQueue.empty())
            dataCond.wait(lock);

        if(disposing)
            return false;

        value = dataQueue.back();
        dataQueue.pop_back();
        return true;
    }

    bool empty() const
    {
        std::unique_lock<std::mutex> lock(sync);
        return dataQueue.empty();
    }

    size_t size() const
    {
       std::lock_guard<std::mutex> lock(sync);
       return dataQueue.size();
    }
private:
    std::deque<T> dataQueue;
    mutable std::mutex sync;
    mutable std::condition_variable dataCond;
    volatile bool disposing;


};


}
}


#endif // GRLX_ASYNC_QUEUE_H
