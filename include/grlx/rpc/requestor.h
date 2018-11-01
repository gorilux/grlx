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
#include <condition_variable>
#include <zmq.hpp>

#include <grlx/service/servicecontainer.h>
#include <grlx/tmpl/signal.h>
#include "serviceactivator.h"
#include "invoker.h"

namespace grlx {
namespace rpc {



template<typename EncoderType, typename Endpoint>
class Requestor : public Invoker<EncoderType, Details::DummyBaseClass >
{

public:
    using EndpointType = Endpoint;
    grlx::Signal<void()>  Connected;
    grlx::Signal<void()>  Disconnected;

    Requestor(ServiceContainerPtr serviceContainer)
        : endpoint( new EndpointType( serviceContainer ))
    {
        hookEvents();
    }

    Requestor()
        : Requestor(ServiceContainerFactory::create())
    {
    }
    virtual ~Requestor()
    {
        this->close();
    }
    void listen(std::string const& addr)
    {
        endpoint->listen(addr);
    }

    void connect(std::string const& addr)
    {
        endpoint->connect(addr);
    }
    void close()
    {
        endpoint->close();
    }

    bool isConnected()
    {
        return connected;
    }

protected:

    void send(const char* data, size_t len, TokenType const& userToken) override
    {
        endpoint->send(data,len, userToken);
    }

private:
    void hookEvents()
    {
        endpoint->MsgReceived.Attach(std::bind(&Requestor::handleResp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        endpoint->Connected.Attach([this](){ connected = true; this->Connected.Emit(); });
        endpoint->Disconnected.Attach([this](){ connected = false; this->Disconnected.Emit(); });
        endpoint->Accepted.Attach([this](){ connected = true; this->Connected.Emit(); });
    }


private:
    std::unique_ptr<EndpointType> endpoint;
    bool connected = false;



};

}
}