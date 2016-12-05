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
#ifndef GRLX_RPC_CLIENT_H
#define GRLX_RPC_CLIENT_H

#include <memory>

#include "connection.h"
#include "serviceprovider.h"

namespace grlx {
namespace rpc {


template<typename EncoderType, typename TransportType >
class Client : public ServiceProvider<EncoderType>, public Connection< typename TransportType::template ClientImpl< Client< EncoderType, TransportType> > >::Type
{
    using BaseType = typename Connection< typename TransportType::template ClientImpl< Client< EncoderType, TransportType> > >::Type;

public:
    using Type = Client;

    using ConnectionType = BaseType;


    template<typename ...TArgs>
    Client(TArgs&&... args)
        : BaseType(std::forward<TArgs>(args)...)
    {
        this->bind(this);
    }
private:


};

}
}

#endif // GRLX_RPC_CLIENT_H
