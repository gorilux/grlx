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

#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <zmq.hpp>

#include <grlx/service/servicecontainer.h>
#include "serviceprovider.h"
#include "invoker.h"

namespace grlx {
namespace rpc {


template<typename EncoderType>
class Client : public Invoker<EncoderType, Details::DummyBaseClass, Client<EncoderType> >
{

    class Poller
    {
    public:
        using HandlerType = std::function<void(zmq::socket_t* socket)>;

        template<typename F>
        Poller (F handler)
            : disposing(false),
              pollThread( std::bind(&Poller::poll, this )),
              handler(handler)


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

        void add(zmq::socket_t* socket, short events)
        {
            std::unique_lock<std::mutex> lock(mtx);

            zmq::pollitem_t pollItem = { static_cast<void*>(socket), 0, ZMQ_POLLIN, 0 };

            pollItems.push_back(pollItem);

            cv.notify_one();
        }

        void remove(zmq::socket_t *socket)
        {

            std::unique_lock<std::mutex> lock(mtx);
            pollItems.erase(std::remove_if(pollItems.begin(), pollItems.end(),[socket]( const auto& item)
            {
                return (static_cast<void*>(socket) == item.socket);
            }), pollItems.end());

            cv.notify_one();
        }

        bool wait(std::chrono::milliseconds timeout)
        {
            int cnt;
            do {

                std::unique_lock<std::mutex> lock(mtx);
                if (pollItems.empty() && !disposing)
                {

                    cv.wait(lock, [this]()->bool
                            {
                                return !this->pollItems.empty() || !this->disposing;
                            });
                }
                if(disposing)
                    return false;

                cnt = zmq::poll(&pollItems[0], pollItems.size(), static_cast<long>(timeout.count()));
                if (cnt == 0)
                    continue;

                auto poIt = pollItems.begin();

                int i = 0;
                while (i < cnt && poIt != pollItems.end())
                {
                    if (poIt->revents & ZMQ_POLLIN)
                    {
                        auto socket = static_cast<zmq::socket_t*>(poIt->socket);
                        handler(socket);
                        i++;
                    }
                    ++poIt;
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
            }catch(...){
            }


        }

    private:
        std::vector<zmq::pollitem_t> pollItems;
        std::mutex mtx;
        std::condition_variable cv;
        bool disposing;
        std::thread pollThread;
        HandlerType handler;

    };

public:
    Client(ServiceContainerPtr parent)
        : serviceContainer( ServiceContainerFactory::create( parent ))
    {
        if( !serviceContainer->hasService<zmq::context_t>() )
        {
            serviceContainer->addService<zmq::context_t>([]()
            {
                return std::make_shared<zmq::context_t>(std::thread::hardware_concurrency());
            });
        }
    }

    Client()
        : Client(ServiceContainerFactory::create())
    {
    }
    virtual ~Client()
    {
        this->disconnect();
    }


    void connect(std::string const& addr)
    {
        this->addr = addr;

        zmq_ctx = serviceContainer->get<zmq::context_t>();

        zmq_socket.reset( new zmq::socket_t( *zmq_ctx, ZMQ_REQ ));

        zmq_socket->connect(addr);

        poller.reset( new Poller(std::bind(&Client::receiveMessage,this, std::placeholders::_1)));

        poller->add(zmq_socket.get(), ZMQ_POLLIN);

    }
    void disconnect()
    {
        poller->remove(zmq_socket.get());
        zmq_socket->disconnect(addr);
        zmq_socket.reset();
    }

protected:
    void send(const char* data, size_t size) override
    {
        zmq::message_t msg(data, size);

        zmq_socket->send(msg);
    }
private:

    void receiveMessage( zmq::socket_t* socket)
    {
        zmq::message_t msg;

        socket->recv(&msg);

        this->handleResp(static_cast<const char*>(msg.data()), msg.size());
    }


private:

    grlx::ServiceContainerPtr serviceContainer;
    std::shared_ptr<zmq::context_t> zmq_ctx;
    std::unique_ptr<zmq::socket_t> zmq_socket;
    std::unique_ptr<Poller> poller;
    std::string addr;

};

}
}
