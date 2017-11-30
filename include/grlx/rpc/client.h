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
#include "serviceprovider.h"
#include "invoker.h"

namespace grlx {
namespace rpc {



template<typename EncoderType, typename Transport>
class Client : public Invoker<EncoderType, Details::DummyBaseClass >
{

public:
    using TransportType = typename Transport::ClientType;

    Client(ServiceContainerPtr serviceContainer)
        : transport( new TransportType( serviceContainer ))
    {
        hookEvents();
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
        transport->connect(addr);

    }
    void disconnect()
    {
        transport->disconnect();
    }
protected:

    void send(const char* data, size_t len) override
    {
        transport->send(data,len);
    }

private:
    void hookEvents()
    {
        transport->MsgReceived.Attach(std::bind(&Client::handleResp, this, std::placeholders::_1, std::placeholders::_2));
    }


private:

    std::unique_ptr<TransportType> transport;



};

}
}
