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
#ifndef GRLX_RPC_CONNECTION_H
#define GRLX_RPC_CONNECTION_H

#include "types.h"

namespace grlx {
namespace rpc {



template<typename ServiceProvider, typename TransportType >
class Connection : public TransportType::template ConnectionImpl< Connection<ServiceProvider, TransportType> >
{

    using BaseType = typename TransportType::template ConnectionImpl< Connection<ServiceProvider, TransportType> >;

public:

    using Type = Connection;


    template<typename ...TArgs>
    Connection(TArgs&&... args)
        : BaseType(std::forward<TArgs>(args)...){}


    void setMsgHandler( MsgHandler handler)
    {
        msgHandler = handler;
    }

private:
    friend BaseType;
    MsgHandler msgHandler;


};

} // namespace rpc
} // namespace grlx

#endif // GRLX_RPC_CONNECTION_H
