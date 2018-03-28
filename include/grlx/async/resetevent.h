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

#include <mutex>
#include <condition_variable>
#include <chrono>

namespace grlx
{

namespace async
{


class ResetEvent
{
public:
    ResetEvent(bool autoReset = false)
        : state( false ), autoReset( autoReset )
    {}
    void set()
    {
        std::unique_lock<std::mutex> lock(mtx);
        state = true;
        cv.notify_one();
    }
    void reset()
    {
        std::unique_lock<std::mutex> lock(mtx);
        state = false;
    }
    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx);

        if(state)
        {
            if(autoReset)
                state = false;
            return;
        }

        cv.wait(lock, [this]() {return state;} );

        if(autoReset)
            state = false;
    }
    template<class Clock, class Duration>
    bool wait_until( const std::chrono::time_point<Clock, Duration>& timeout_time)
    {
        std::unique_lock<std::mutex> lock(mtx);

        if(state)
        {
            if(autoReset)
                state = false;
            return true;
        }

        bool ret = cv.wait_until(lock, timeout_time, [this]() {return state;} );

        if(ret && state)
        {
            if(autoReset)
                state = false;
            return true;
        }
        return ret;
    }

    template< class Rep, class Period>
    bool wait_for( const std::chrono::duration<Rep, Period>& rel_time)
    {
        std::unique_lock<std::mutex> lock(mtx);

        if(state)
        {
            if(autoReset)
                state = false;
            return true;
        }

        bool ret = cv.wait_for(lock, rel_time, [this]() {return state;} );

        if(ret && state)
        {
            if(autoReset)
                state = false;
            return true;
        }
        return ret;
    }
private:
    bool state;
    bool autoReset;
    std::condition_variable cv;
    std::mutex mtx;

};


}

}

