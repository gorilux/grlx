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
#ifndef GRLX_RPC_SERVER_H
#define GRLX_RPC_SERVER_H

#include <memory>

#include "serviceprovider.h"

namespace grlx {
namespace rpc {

template<typename ServiceProvider, typename TransportType >
class Server : public TransportType::template ServerImpl< Server<ServiceProvider, TransportType> >
{

    using BaseType = typename TransportType::template ServerImpl< Server<ServiceProvider, TransportType> >;
    using ConnectionType = typename TransportType::Connection;

public:

    template<typename ...TArgs>
    Server(TArgs&&... args)
        : BaseType(std::forward<TArgs>(args)...)
    {
        createServiceDelegate = [] (ConnectionType* connection)
        {
            return std::make_shared<ServiceProvider>(connection);
        };
    }

private:
    friend BaseType;

    template<typename ConnectionType>
    bool accept(std::shared_ptr<ConnectionType>&& newConnection)
    {



        return true;
    }


    std::function< std::shared_ptr<ServiceProvider>(ConnectionType*) > createServiceDelegate;




};

} // namespace rpc
} // namespace grlx

#endif // GRLX_RPC_SERVER_H
