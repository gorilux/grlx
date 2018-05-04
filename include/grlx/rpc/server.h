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
#include <unordered_map>
#include <type_traits>

#include <grlx/service/servicecontainer.h>
#include <grlx/tmpl/signal.h>

#include "serviceprovider.h"

namespace grlx
{
namespace rpc
{


template<typename EncoderType, typename Transport>
class Server : public ServiceProvider<EncoderType, Details::DummyBaseClass >
{

public:

    using TransportType = Transport;
    grlx::Signal<void()>  Connected;
    grlx::Signal<void()>  Disconnected;
    grlx::Signal<void()>  Listening;
    grlx::Signal<void()>  BindFailed;
    grlx::Signal<void()>  Accepted;
    grlx::Signal<void()>  Closed;



    Server(ServiceContainerPtr serviceContainer)
        : transport( new TransportType( serviceContainer ))
    {
        hookEvents();
    }

    Server()
        : Server(ServiceContainerFactory::create())
    {
    }
    virtual ~Server()
    {
        this->close();
    }
    void open(std::string const& addr)
    {
        transport->open(addr);

    }
    void close()
    {
        transport->close();
    }

    bool isConnected()
    {
        return connected;
    }


protected:

    void send(const char* data, size_t len) override
    {
        transport->send(data,len);
    }

private:
    void hookEvents()
    {
        transport->MsgReceived.Attach(std::bind(&Server::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
        transport->Connected.Attach([this](){ connected = true; this->Connected.Emit(); });
        transport->Disconnected.Attach([this](){ connected = false; this->Disconnected.Emit(); });
        transport->Listening.Attach([this](){ this->Listening.Emit(); });
        transport->BindFailed.Attach([this](){ this->BindFailed.Emit(); });
        transport->Accepted.Attach([this](){ this->Accepted.Emit(); });
        transport->Closed.Attach([this](){ this->Closed.Emit(); });

    }


private:
    std::unique_ptr<TransportType> transport;
    bool connected = false;




};

} // namespace rpc
} // namespace grlx
