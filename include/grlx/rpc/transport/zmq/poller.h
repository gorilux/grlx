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
#include <functional>
#include <grlx/service/servicecontainer.h>

#include <zmq.hpp>



namespace grlx
{

namespace rpc
{

namespace ZeroMQ
{

class Poller
{
public:

    using HandlerType = std::function<void(zmq::socket_t* socket)>;

    Poller( grlx::ServiceContainerPtr serviceContainer );

    ~Poller ();

    void add(zmq::socket_t* socket, short events, HandlerType handler);

    void remove(zmq::socket_t *socket);

private:

    class Private;
    friend class Private;
    std::unique_ptr<Private> d_ptr;
};


}

}

}
