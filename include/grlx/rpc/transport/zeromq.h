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
#include <list>
#include <mutex>
#include <thread>



#include <zmq.hpp>

#include "zmq/poller.h"

#include <grlx/tmpl/signal.h>
#include <grlx/service/servicecontainer.h>

#include <iostream>


namespace grlx {

namespace rpc {

namespace ZeroMQ
{

template<bool Bindable>
class ZMQSocketImpl
{

public:
    static void open(zmq::socket_t* socket, std::string const& addr)
    {
        socket->bind(addr);
    }
    static void close(zmq::socket_t* socket, std::string const& addr)
    {
        socket->unbind(addr);
    }
};

template<>
class ZMQSocketImpl<false>
{
public:
    static void open(zmq::socket_t* socket, std::string const& addr)
    {
        socket->connect(addr);
    }
    static void close(zmq::socket_t* socket, std::string const& addr)
    {
        socket->disconnect(addr);
    }

};


template<zmq::socket_type SocketType, bool IsServer>
class ZMQSocket
{
private:
    using Impl = ZMQSocketImpl<IsServer>;
public:
    using PollerType = Poller;

    grlx::Signal<void(const char*, size_t)> MsgReceived;
    grlx::Signal<void()>                    Connected;
    grlx::Signal<void()>                    Disconnected;
    grlx::Signal<void()>                    ConnectDelayed;
    grlx::Signal<void()>                    ConnectRetried;
    grlx::Signal<void()>                    Listening;
    grlx::Signal<void()>                    BindFailed;
    grlx::Signal<void()>                    Accepted;
    grlx::Signal<void()>                    AcceptFailed;
    grlx::Signal<void()>                    Closed;
    grlx::Signal<void()>                    CloseFailed;
    grlx::Signal<void()>                    HandshakeFailed;
    grlx::Signal<void()>                    HandshakeSucceed;
    grlx::Signal<void()>                    Unknown;


    ZMQSocket( grlx::ServiceContainerPtr serviceContainer)
        : serviceContainer(serviceContainer)
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
            serviceContainer->addService<PollerType>([serviceContainer]()
            {
                return std::make_shared<PollerType>(serviceContainer);
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


        this->attachMonitor(*zmq_socket, ZMQ_EVENT_ALL);

        Impl::open(zmq_socket.get(), addr);

        auto poller = serviceContainer->get<PollerType>();

        poller->add(zmq_socket.get(), ZMQ_POLLIN, [this](zmq::socket_t* socket)
        {

            zmq::message_t msg;
            socket->recv(&msg);
//            std::cout.write(static_cast<const char*>(msg.data()), msg.size());
//            std::cout << std::endl;
            this->MsgReceived.Emit(static_cast<const char*>(msg.data()), msg.size());

        });


    }

    void close()
    {
        if(!zmq_socket)
        {
            return;
        }

        auto poller = serviceContainer->get<PollerType>();

        poller->remove(zmq_socket.get());

        Impl::close(zmq_socket.get(), addr);

        dettachMonitor();

        zmq_socket.reset();

    }

    void send( const char* data, size_t size )
    {
        zmq::message_t msg(data, size);
        zmq_socket->send(msg);
    }



protected:

    virtual void on_monitor_started()
    {        
    }
    virtual void on_event_connected(const zmq_event_t &event_, std::string const& addr_)
    {   
        this->Connected.Emit();
    }
    virtual void on_event_connect_delayed(const zmq_event_t &event_, std::string const& addr_)
    {
        this->ConnectDelayed.Emit();
    }
    virtual void on_event_connect_retried(const zmq_event_t &event_, std::string const& addr_)
    {
        this->ConnectRetried.Emit();
    }
    virtual void on_event_listening(const zmq_event_t &event_, std::string const& addr_)
    {
        this->Listening.Emit();
    }
    virtual void on_event_bind_failed(const zmq_event_t &event_, std::string const& addr_)
    {
        this->BindFailed.Emit();
    }
    virtual void on_event_accepted(const zmq_event_t &event_, std::string const& addr_)
    {
        this->Accepted.Emit();
    }
    virtual void on_event_accept_failed(const zmq_event_t &event_, std::string const& addr_)
    {
        this->AcceptFailed.Emit();
    }
    virtual void on_event_closed(const zmq_event_t &event_, std::string const& addr_)
    {
        this->Closed.Emit();
    }
    virtual void on_event_close_failed(const zmq_event_t &event_, std::string const& addr_)
    {
        this->CloseFailed.Emit();
    }
    virtual void on_event_disconnected(const zmq_event_t &event_, std::string const& addr_)
    {
        this->Disconnected.Emit();
    }
    virtual void on_event_handshake_failed(const zmq_event_t &event_, std::string const& addr_)
    {
        this->HandshakeFailed.Emit();
    }
    virtual void on_event_handshake_succeed(const zmq_event_t &event_, std::string const& addr_)
    {
        this->HandshakeSucceed.Emit();
    }
    virtual void on_event_unknown(const zmq_event_t &event_, std::string const& addr_)
    {
        this->Unknown.Emit();
    }



private:
    void attachMonitor(zmq::socket_t &socket, int events = ZMQ_EVENT_ALL)
    {
        void* ptr = static_cast<void*>(socket);


        std::string monitorAddr = "inproc://" + std::to_string(reinterpret_cast<std::intptr_t>(ptr))
                + ".monitor";

        int rc = zmq_socket_monitor(ptr, monitorAddr.c_str(), events);
        if (rc != 0)
            throw error_t ();


        auto poller = serviceContainer->get<Poller>();

        auto zmq_ctx = serviceContainer->get<zmq::context_t>();

        zmq_monitor_socket.reset( new zmq::socket_t( *zmq_ctx, ZMQ_PAIR ));

        zmq_monitor_socket->connect(monitorAddr);

        poller->add(zmq_monitor_socket.get(), ZMQ_POLLIN, [this](zmq::socket_t* socket)
        {
            zmq::message_t msg;
            socket->recv(&msg);
            zmq_event_t msgEvent;

            auto data = static_cast<const char*>(msg.data());
            memcpy(&msgEvent.event, data, sizeof(uint16_t));
            data += sizeof(uint16_t);
            memcpy(&msgEvent.value, data, sizeof(int32_t));

            zmq::message_t addrMsg;
            socket->recv(&addrMsg);

            const char* str = static_cast<const char*>(addrMsg.data());
            std::string address(str, str + addrMsg.size());

//#ifdef ZMQ_EVENT_MONITOR_STOPPED
//            if (msgEvent.event == ZMQ_EVENT_MONITOR_STOPPED)
//            {
//                zmq_monitor_socket->close();
//                return;
//            }
//#endif

            switch (msgEvent.event)
            {
            case ZMQ_EVENT_CONNECTED:
                on_event_connected(msgEvent, address);
                break;
            case ZMQ_EVENT_CONNECT_DELAYED:
                on_event_connect_delayed(msgEvent, address);
                break;
            case ZMQ_EVENT_CONNECT_RETRIED:
                on_event_connect_retried(msgEvent, address);
                break;
            case ZMQ_EVENT_LISTENING:
                on_event_listening(msgEvent, address);
                break;
            case ZMQ_EVENT_BIND_FAILED:
                on_event_bind_failed(msgEvent, address);
                break;
            case ZMQ_EVENT_ACCEPTED:
                on_event_accepted(msgEvent, address);
                break;
            case ZMQ_EVENT_ACCEPT_FAILED:
                on_event_accept_failed(msgEvent, address);
                break;
            case ZMQ_EVENT_CLOSED:
                on_event_closed(msgEvent, address);
                break;
            case ZMQ_EVENT_CLOSE_FAILED:
                on_event_close_failed(msgEvent, address);
                break;
            case ZMQ_EVENT_DISCONNECTED:
                on_event_disconnected(msgEvent, address);
                break;
#ifdef ZMQ_BUILD_DRAFT_API
            case ZMQ_EVENT_HANDSHAKE_FAILED:
                on_event_handshake_failed(*event, address);
                break;
            case ZMQ_EVENT_HANDSHAKE_SUCCEED:
                on_event_handshake_succeed(*event, address);
                break;
#endif
            default:
                on_event_unknown(msgEvent, address);
                break;
            }

        });



        on_monitor_started();
    }

    void dettachMonitor()
    {
        if (zmq_monitor_socket)
        {

            auto poller = serviceContainer->get<Poller>();            

            zmq_socket_monitor(static_cast<void*>(*zmq_monitor_socket), nullptr, 0);
            poller->remove(zmq_monitor_socket.get());

            zmq_monitor_socket.reset();
        }
    }
private:

    grlx::ServiceContainerPtr serviceContainer;
    std::shared_ptr<zmq::context_t> zmq_ctx;
    std::unique_ptr<zmq::socket_t> zmq_socket;
    std::unique_ptr<zmq::socket_t> zmq_monitor_socket;
    std::string addr;    

};



using SocketType = zmq::socket_type;

template<SocketType SockT, bool IsServer>
using Socket = ZMQSocket<SockT,IsServer>;

} // zeromq


} // rpc


}
