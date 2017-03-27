/*
*    Copyright (c) 2011 David Salvador Pinheiro
*
*    http://www.gorilux.org
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GRLX_TMPL_SIGNAL_H_
#define GRLX_TMPL_SIGNAL_H_

#include <list>
#include <map>
#include <mutex>
#include <functional>
#include <algorithm>
#include <utility>

#include <grlx/tmpl/reference.h>
#include <grlx/tmpl/pointer.h>
#include <grlx/tmpl/foreach.h>
#include <grlx/tmpl/stlext.h>

namespace grlx
{

template<typename Sig>
struct Signal;

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
        connections.insert( slotId++, std::forward<Slot>(slot) );
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
        std::lock_guard<std::mutex> lock(mutex);
        for( auto& connection: connections)
        {
            connection.second( std::forward<ArgTypes>(args)... );
        }
    }

    template<typename ...ArgTypes>
    void Emit(const ArgTypes&... args)
    {
        std::lock_guard<std::mutex> lock(mutex);
        for( auto& connection: connections)
        {
            connection.second( std::forward<const ArgTypes& >(args)... );
        }
    }

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
