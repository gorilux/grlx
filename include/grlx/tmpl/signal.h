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


#ifndef GRLX_TMPL_SIGNAL_H_
#define GRLX_TMPL_SIGNAL_H_

#include <map>
#include <mutex>
#include <functional>


namespace grlx
{

template<typename Sig>
class Signal;

template<typename R, typename ...Args>
class Signal<R(Args...)>
{
public:

    using Key = unsigned long;
    Signal()
        : slotId(0)
    {
    }
    ~Signal()
    {
    }


    template<typename Slot>
    Key Attach( Slot&& slot )
    {
        std::lock_guard<std::mutex> lock(mutex);
        connections.insert( std::make_pair(++slotId, std::forward<Slot>(slot)) );
        return slotId;
    }

    bool Detach(Key slot)
    {
        std::lock_guard<std::mutex> lock(mutex);
        return connections.erase( slot ) == 1;

    }

    template<typename Slot>
    void operator+= (Slot&& slot)
    {
        Attach( std::forward<Slot>(slot) );
    }

    template<typename ...ArgTypes>
    void Emit(ArgTypes&&... args)
    {
        //std::lock_guard<std::mutex> lock(mutex);
        for( auto& connection: connections)
        {
            connection.second( std::forward<ArgTypes>(args)... );
        }
    }

//    template<typename ...ArgTypes>
//    void Emit(const ArgTypes&... args)
//    {
//        std::lock_guard<std::mutex> lock(mutex);
//        for( auto& connection: connections)
//        {
//            connection.second( std::forward<const ArgTypes& >(args)... );
//        }
//    }

private:

    std::map< Key, std::function<R(Args...)> > connections;
    Key slotId;
    std::mutex mutex;

};


// template<typename ...Args>
// struct Event : Signal<void ( const Args&...) >
// {

// };

// template<>
// struct Event<void> : Signal<void()>
// {

// };

}

#endif
