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
#include <string>
#include <typeinfo>
#include <cstdlib>
#include <thread>

#include <zmq.hpp>

#include "poller.h"

#include <grlx/tmpl/signal.h>
#include <grlx/service/servicecontainer.h>
#include <grlx/rpc/types.h>


//#include <cxxabi.h>



namespace grlx {
namespace rpc {
namespace ZeroMQ {


//inline std::string demangle(const char* name) {

//    int status = -4; // some arbitrary value to eliminate the compiler warning

//    // enable c++11 by passing the flag -std=c++11 to g++
//    std::unique_ptr<char, void(*)(void*)> res {
//        abi::__cxa_demangle(name, NULL, NULL, &status),
//        std::free
//    };

//    return (status==0) ? res.get() : name ;
//}

//template <class T>
//std::string type(const T& t) {

//    return demangle(typeid(t).name());
//}



class ZMQSocket
{
public:
    grlx::Signal<void(const char*, size_t, TokenType const&)> MsgReceived;
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


    ZMQSocket(zmq::socket_type socketType, grlx::ServiceContainerPtr serviceContainer);
    virtual ~ZMQSocket();

    void connect(std::string const& addr);

    void disconnect(std::string const& addr);

    void bind(std::string const& addr);

    void listen(std::string const& addr);

    void unbind(std::string const& addr);

    void send( const char* data, size_t size );

    void close();

    template<typename T> void setSockOpt(int option_, T const& optval)
    {
        setSockOpt(option_, &optval, sizeof(T) );
    }

    void setSockOpt (int option_, const void *optval_, size_t optvallen_);

    void getSockOpt (int option_, void *optval_, size_t *optvallen_) const;

    template<typename T> T getSockOpt(int option_) const
    {
        T optval;
        size_t optlen = sizeof(T);
        getSockOpt(option_, &optval, &optlen );
        return optval;
    }

protected:

    virtual void onMonitorStarted();
    virtual void onEventConnected(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventConnectDelayed(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventConnectRetried(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventListening(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventBindFailed(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventAccepted(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventAcceptFailed(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventClosed(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventCloseFailed(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventDisconnected(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventHandshakeFailed(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventHandshakeSucceed(const zmq_event_t &event_, std::string const& addr_);
    virtual void onEventUnknown(const zmq_event_t &event_, std::string const& addr_);
    virtual void onDataAvailable();
    virtual bool receiveMsg(zmq::message_t *msg, int flags = 0);
    virtual size_t sendMsg(zmq::message_t& msg, int flags = 0);

private:
    class FSM;
    friend class FSM;
    std::unique_ptr<FSM> fsm;

};

} // namespace ZeroMQ
} // namespace rpc
} // namespace grlx

