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


#include "poller.h"

#include <grlx/async/resetevent.h>
#include <thread>
#include <mutex>
#include <map>
#include <list>
#include <atomic>

#include <iostream>


namespace grlx
{

namespace rpc
{

namespace ZeroMQ
{

class Poller::Private
{
public:

    enum class EventType
    {
        SOCKET_ADDED = 1,
        SOCKET_REMOVED,
        DISPOSING
    };

    Private( grlx::ServiceContainerPtr serviceContainer, Poller* q)
        : serviceContainer( serviceContainer ),
          disposing(false),
          q_ptr( q )
    {
        if( !serviceContainer->hasService<zmq::context_t>() )
        {
            serviceContainer->addService<zmq::context_t>([]()
            {
                return std::make_shared<zmq::context_t>(std::thread::hardware_concurrency());
            });
        }

        init();
    }
    ~Private()
    {
        disposing = true;
        postEvent(EventType::DISPOSING);
        pollingThread.join();
    }

    void init()
    {
        internalAddr = "inproc://" + std::to_string(reinterpret_cast<std::intptr_t>(static_cast<void*>(this)))
                                               + ".poller";

        auto zmq_ctx = serviceContainer->get<zmq::context_t>();

        internalSocket.reset(new zmq::socket_t(*zmq_ctx, ZMQ_PAIR));

        internalSocket->bind(internalAddr);

        pollerSocket.reset(new zmq::socket_t(*zmq_ctx, ZMQ_PAIR));

        pollerSocket->connect(internalAddr);

        void* ptr = static_cast<void*>(*internalSocket);
        sockets[ptr] = std::make_pair(internalSocket.get(), [](zmq::socket_t*) {});

        zmq::pollitem_t pollItem = { ptr, 0, ZMQ_POLLIN, 0 };
        pollItems.push_back(std::move(pollItem));

        pollingThread = std::thread( std::bind(&Private::pollSockets, this) );

        pollingStarted.wait();


    }

    void add(zmq::socket_t* socket, short events, Poller::HandlerType handler)
    {
        std::lock_guard<std::mutex> lock(mtx);

        void* ptr = static_cast<void*>(*socket);
        sockets[ptr] = std::make_pair(socket,handler);
        postEvent(EventType::SOCKET_ADDED);
    }
    void remove(zmq::socket_t* socket)
    {
        std::lock_guard<std::mutex> lock(mtx);

        void *ptr = static_cast<void*>(*socket);
        sockets.erase( ptr );

        postEvent(EventType::SOCKET_REMOVED);

    }

    EventType readEvent(zmq::socket_t* socket)
    {
        zmq::message_t msg;
        socket->recv(&msg);
        zmq_event_t msgEvent;

        auto data = static_cast<const char*>(msg.data());
        memcpy(&msgEvent.event, data, sizeof(uint16_t));
        data += sizeof(uint16_t);
        memcpy(&msgEvent.value, data, sizeof(int32_t));

        return static_cast<EventType>(msgEvent.event);
    }

    void postEvent(EventType event)
    {
        char buffer[sizeof(uint16_t) + sizeof(int32_t)];

        auto e = reinterpret_cast<const char*>(&event);
        memcpy(buffer, e, sizeof(uint16_t));

        int32_t value = 0;
        auto v = reinterpret_cast<const char*>(&value);
        memcpy(buffer+sizeof(uint16_t), v, sizeof(int32_t));

        zmq::message_t msg(buffer, sizeof(buffer));

        pollerSocket->send(msg);

    }


    void pollSockets()
    {

        pollingStarted.set();

        while(!disposing)
        {
            int cnt = 0;            
            try
            {
                cnt = zmq::poll(&pollItems[0], pollItems.size(), -1);
            }
            catch(zmq::error_t const& e)
            {
                //std::cout << "poll exception: " << e.what() << std::endl;
                rebuildPollItems();
                continue;
            }



            if (cnt >= 0)
            {
                bool shouldRebuildPollItems = false;
                //auto poIt = pollItems.begin();
                int i = 0;
                for( auto poIt = pollItems.begin(); poIt != pollItems.end(); poIt++)
                {
                    if (poIt->revents & ZMQ_POLLIN)
                    {
                        auto& item = *poIt;
                        auto socketPtr = item.socket;
                        if(socketPtr == static_cast<void*>(*internalSocket))
                        {
                            switch(readEvent(internalSocket.get()))
                            {
                                case EventType::SOCKET_ADDED:
                                    shouldRebuildPollItems = true;
                                    break;
                                case EventType::SOCKET_REMOVED:
                                    shouldRebuildPollItems = true;
                                    break;
                                case EventType::DISPOSING:
                                    return;
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            auto itr = sockets.find(socketPtr);
                            if( itr != sockets.end())
                            {
                                auto socket = itr->second.first;
                                auto handler = itr->second.second;
                                try
                                {
                                    handler( socket );

                                }
                                catch(zmq::error_t const& e)
                                {
                                    std::cout << "exception: " << e.what() << std::endl;
                                }

                            }
                        }
                        i++;
                    }
                }
                if(shouldRebuildPollItems)
                {
                    rebuildPollItems();
                }

            }

        }

    }

    void rebuildPollItems()
    {
        pollItems.clear();
        std::lock_guard<std::mutex> lock(mtx);
        for(auto& pair: sockets)
        {
            zmq::pollitem_t pollItem = { pair.first, 0, ZMQ_POLLIN, 0 };
            pollItems.push_back(std::move(pollItem));
        }

    }

    std::map<void*, std::pair<zmq::socket_t*, HandlerType> > sockets;
    std::vector<zmq::pollitem_t> pollItems;
    mutable std::mutex mtx;
    grlx::async::ResetEvent pollingCompleted;
    grlx::async::ResetEvent pollingStarted;
    std::string internalAddr;
    std::thread pollingThread;
    std::unique_ptr<zmq::socket_t> pollerSocket;
    std::unique_ptr<zmq::socket_t> internalSocket;

    grlx::ServiceContainerPtr serviceContainer;
    std::atomic<bool> disposing;
private:
    Poller *q_ptr;
};

Poller::Poller(grlx::ServiceContainerPtr serviceContainer)
    : d_ptr( new Private(serviceContainer, this) )
{

}

Poller::~Poller()
{

}

void Poller::add(zmq::socket_t* socket, short events, Poller::HandlerType handler)
{
    d_ptr->add(socket, events, handler);
}

void Poller::remove(zmq::socket_t* socket)
{
    d_ptr->remove(socket);
}


}

}

}
