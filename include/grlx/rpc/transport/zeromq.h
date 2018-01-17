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

#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <zmq.hpp>

#include <grlx/tmpl/signal.h>
#include <grlx/service/servicecontainer.h>

#include <iostream>


namespace grlx {

namespace rpc {

namespace ZeroMQ
{


class Poller
{
public:
    using HandlerType = std::function<void(zmq::socket_t* socket)>;

    Poller()
        : disposing(false),
          pollThread( std::bind(&Poller::poll, this ))
    {

    }

    ~Poller ()
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            disposing = true;
            cv.notify_all();
        }

        if(pollThread.joinable())
            pollThread.join();

    }

    void add(zmq::socket_t* socket, short events, HandlerType handler)
    {
        std::unique_lock<std::mutex> lock(mtx);
        sockets[static_cast<void*>(*socket)] = std::make_pair(socket,handler);
        cv.notify_one();
    }

    void remove(zmq::socket_t *socket)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            sockets.erase( static_cast<void*>(*socket) );
            cv.notify_one();
        }
        {
            std::unique_lock<std::mutex> lock(pollingMtx);
            if(!pollItems.empty())
                pollingFinished.wait(lock);
        }

    }

    bool wait(std::chrono::milliseconds timeout)
    {
        int cnt;
        do {

            {
                std::unique_lock<std::mutex> lock(mtx);
                if (sockets.empty() && !disposing)
                {
                    cv.wait(lock, [this]()->bool
                    {
                        return !this->sockets.empty() || !this->disposing;
                    });
                }

                if(disposing)
                    return false;

                if(pollItems.size() != sockets.size())
                {
                    pollItems.clear();
                    for(auto& pair: sockets)
                    {
                        zmq::pollitem_t pollItem = { pair.first, 0, ZMQ_POLLIN, 0 };
                        pollItems.push_back(std::move(pollItem));
                    }
                }
            }

            try
            {
                {
                    std::unique_lock<std::mutex> lock(pollingMtx);
                    if(!pollItems.empty())
                    {
                        cnt = zmq::poll(&pollItems[0], pollItems.size(), static_cast<long>(timeout.count()));
                    }
                    else
                    {
                        cnt = 0;
                    }
                    pollingFinished.notify_all();
                    if (cnt == 0)
                    {
                        continue;
                    }
                }

                auto poIt = pollItems.begin();

                int i = 0;
                while (i < cnt && poIt != pollItems.end())
                {
                    if (poIt->revents & ZMQ_POLLIN)
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        auto socketPtr = poIt->socket;
                        auto itr = sockets.find(socketPtr);
                        if( itr != sockets.end())
                        {
                            auto socket = itr->second.first;
                            auto handler = itr->second.second;
                            handler( socket );
                        }
                        i++;
                    }
                    ++poIt;
                }
            }
            catch(zmq::error_t const& e)
            {
                std::cout << "exception: " << e.what() << std::endl;
                continue;
            }
        } while (cnt > 0);
        return true;
    }
private:
    void poll()
    {

        try {

            while(wait(std::chrono::milliseconds(100)))
            {
            }
        }
        catch(zmq::error_t const& e )
        {

        }



    }

private:
    std::vector<zmq::pollitem_t> pollItems;
    std::map<void*, std::pair<zmq::socket_t*, HandlerType> > sockets;
    std::mutex mtx;
    std::mutex pollingMtx;
    std::condition_variable cv;
    std::condition_variable pollingFinished;
    bool disposing;
    std::thread pollThread;

};




template<bool Bindable>
class ZMQSocketImpl
{

protected:

    void open(zmq::socket_t* socket, std::string const& addr)
    {
        socket->bind(addr);
    }
    void close(zmq::socket_t* socket, std::string const& addr)
    {
        socket->unbind(addr);
    }
};

template<>
class ZMQSocketImpl<false>
{
protected:
    void open(zmq::socket_t* socket, std::string const& addr)
    {
        socket->connect(addr);
    }
    void close(zmq::socket_t* socket, std::string const& addr)
    {
        socket->disconnect(addr);
    }

};


template<zmq::socket_type SocketType, bool IsServer>
class ZMQSocket : public ZMQSocketImpl<IsServer>
{
private:
    using Impl = ZMQSocketImpl<IsServer>;
public:
    using PollerType = Poller;

    grlx::Signal<void(const char*, size_t)> MsgReceived;

    ZMQSocket( grlx::ServiceContainerPtr serviceContainer)
        : serviceContainer( serviceContainer)
    {
        if( !serviceContainer->hasService<zmq::context_t>() )
        {
            serviceContainer->addService<zmq::context_t>([]()
            {
                return std::make_shared<zmq::context_t>(std::thread::hardware_concurrency());
            });
        }
        if( !serviceContainer->hasService< PollerType > ())
        {
            serviceContainer->addService<PollerType>([]()
            {
                return std::make_shared<PollerType>();
            });
        }
    }

    virtual ~ZMQSocket()
    {
        this->close();
    }

    void open(std::string const& addr)
    {
        this->addr = addr;

        zmq_ctx = serviceContainer->get<zmq::context_t>();

        if( zmq_socket )
            close();

        zmq_socket.reset( new zmq::socket_t( *zmq_ctx, SocketType ));

        Impl::open(zmq_socket.get(), addr);

        auto poller = serviceContainer->get<PollerType>();

        poller->add(zmq_socket.get(), ZMQ_POLLIN, [this](zmq::socket_t* socket)
        {
            zmq::message_t msg;
            socket->recv(&msg);
            this->MsgReceived.Emit(static_cast<const char*>(msg.data()), msg.size());
        });

    }

    void close()
    {
        if(!zmq_socket)
            return;

        auto poller = serviceContainer->get<PollerType>();

        poller->remove(zmq_socket.get());

        Impl::close(zmq_socket.get(), addr);

        zmq_socket.reset();
    }

    void send( const char* data, size_t size )
    {
        zmq::message_t msg(data, size);
        zmq_socket->send(msg);
    }

private:

    grlx::ServiceContainerPtr serviceContainer;
    std::shared_ptr<zmq::context_t> zmq_ctx;
    std::unique_ptr<zmq::socket_t> zmq_socket;
    std::string addr;

};



using SocketType = zmq::socket_type;

template<SocketType SockT, bool IsServer>
using Socket = ZMQSocket<SockT,IsServer>;

} // zeromq


} // rpc


}
