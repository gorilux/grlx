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
#include <unordered_map>
#include <type_traits>

#include "connection.h"
#include "serviceprovider.h"

namespace grlx {
namespace rpc {
namespace details {

template<typename T>
class IsDefaultConstructible
{

    typedef char yes;
    typedef struct { char arr[2]; } no;

    template<typename U>
    static decltype(U(), yes()) test(int);

    template<typename>
    static no test(...);

public:

    static const bool value = sizeof(test<T>(0)) == sizeof(yes);
};



}


template<typename EncoderType, typename TransportType >
class Server : public ServiceProvider< EncoderType , typename TransportType::template ServerImpl< Server< EncoderType , TransportType> > >::Type
{

    using BaseType = typename ServiceProvider< EncoderType, typename TransportType::template ServerImpl< Server< EncoderType, TransportType> > >::Type;
    using Impl = typename TransportType::template ServerImpl< Server< EncoderType , TransportType> >;

public:
    using Type = Server;
    using ConnectionType = typename Connection<typename BaseType::ConnectionType>::Type;
    using CreateServiceDelegate = std::function< std::shared_ptr< ServiceProvider<EncoderType> >() >;

    template<typename ...TArgs>
    Server(TArgs&&... args)
        : BaseType(std::forward<TArgs>(args)...),
          disposing(false)
    {

    }

    ~Server()
    {
        disposing = true;

        dispose();
    }
private:
    friend Impl;
    friend ConnectionType;

    void disconnected(std::shared_ptr<ConnectionType> const& connection)
    {
        if(disposing)
            return;
    }
    void dispose()
    {
        for(auto connection: outstandingConnections)
        {
            connection->close();
        }
    }

    bool accept(std::shared_ptr<ConnectionType> const& newConnection)
    {

        this->bind(newConnection.get());

        outstandingConnections.push_back( newConnection );

        return true;
    }

    void handleClosed()
    {
        outstandingConnections.clear();
    }

private:

    std::list< std::shared_ptr<ConnectionType> > outstandingConnections;
    bool disposing;




};

} // namespace rpc
} // namespace grlx

#endif // GRLX_RPC_SERVER_H
