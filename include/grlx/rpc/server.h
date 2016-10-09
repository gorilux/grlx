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


template<typename ServiceProvider, typename TransportType >
class Server : public TransportType::template ServerImpl< Server<ServiceProvider, TransportType> >
{

    using BaseType = typename TransportType::template ServerImpl< Server<ServiceProvider, TransportType> >::Type;

public:
    using Type = Server;
    using ConnectionType = typename Connection<typename BaseType::ConnectionType>::Type;
    using CreateServiceDelegate = std::function< std::shared_ptr< ServiceProvider>() >;

    template<typename ...TArgs>
    Server(TArgs&&... args)
        : BaseType(std::forward<TArgs>(args)...),
          disposing(false)
    {
        checkAndAssignDefaultFactory<ServiceProvider>();
    }

    void setServiceFactory( CreateServiceDelegate serviceFactory)
    {
        createServiceDelegate = serviceFactory;
    }



    ~Server()
    {
        disposing = true;

        dispose();

    }
private:
    friend BaseType;
    friend ConnectionType;


    template<typename T>
    typename std::enable_if<details::IsDefaultConstructible<T>::value>::type
    checkAndAssignDefaultFactory()
    {
        createServiceDelegate = [] ()
            {
                return std::make_shared<T>();
            };
    }
    template<typename T>
    typename std::enable_if<!details::IsDefaultConstructible<T>::value>::type
    checkAndAssignDefaultFactory(){ }


    void disconnected(std::shared_ptr<ConnectionType> const& connection)
    {
        if(disposing)
            return;

        auto serviceItr = activeServices.find( connection );
        if(serviceItr != activeServices.end())
        {
            //serviceItr->second->cancel();
            activeServices.erase(serviceItr);
        }
    }
    void dispose()
    {
        for(auto service: activeServices)
        {
            service.first->close();
        }
    }

    bool accept(std::shared_ptr<ConnectionType> const& newConnection)
    {

        auto newService = createServiceDelegate();

        newService->bind(newConnection.get());

        activeServices.insert(std::make_pair( newConnection, newService));

        return true;
    }

    void handleClosed()
    {
        activeServices.clear();
    }



private:
    std::function< std::shared_ptr< ServiceProvider>() > createServiceDelegate;
    std::unordered_map< std::shared_ptr<ConnectionType>, std::shared_ptr<ServiceProvider > > activeServices;
    bool disposing;




};

} // namespace rpc
} // namespace grlx

#endif // GRLX_RPC_SERVER_H
