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

#include <grlx/service/servicecontainer.h>
#include <grlx/rpc/transport/zeromq.h>


namespace grlx {
namespace rpc {
namespace ZeroMQ {


class DealerEndpoint  : public ZMQSocket
{
public:
    DealerEndpoint(grlx::ServiceContainerPtr serviceContainer);
    DealerEndpoint(const std::string& identity, grlx::ServiceContainerPtr serviceContainer);

    virtual ~DealerEndpoint();


    void setIdentity(std::string const& value);

    void send(const char* data, size_t size, TokenType const& userToken);

    // ZMQSocket interface
protected:
    void onDataAvailable() override;

private:
    class FSM;
    friend class FSM;
    std::unique_ptr<FSM> fsm;

};

} // namespace ZeroMQ
} // namespace rpc
} // namespace grlx

