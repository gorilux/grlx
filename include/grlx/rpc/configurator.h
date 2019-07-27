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


#include <functional>
#include <grlx/service/servicecontainer.h>
#include <grlx/rpc/exceptions.h>

namespace grlx
{
namespace rpc
{


template<typename T>
struct ConfiguratorImpl;


template<typename T>
struct Configurator : ConfiguratorImpl< T >{};


class Configuration
{
public:

    static const int TIMEOUT = 30*1000;

    template<typename TService, typename ServiceProviderT, typename TServiceCreator>
    static void registerService(std::string const& serviceId, ServiceProviderT& serviceProvider, TServiceCreator&& serviceCreator, ServiceContainerPtr serviceContainer = grlx::ServiceContainer::globalInstance())
    {
        auto serviceInstance = std::invoke(serviceCreator);

        serviceContainer->addService<TService>(serviceInstance);

        Configurator<TService>::configure(serviceId, serviceProvider, *serviceInstance);
    }

    template<typename TService, typename InvokerT>
    static typename Configurator<TService>::Remote remoteInstance(std::string const& serviceId, InvokerT* invokerType, ServiceContainerPtr serviceContainer = grlx::ServiceContainer::globalInstance())
    {
        return std::make_shared<typename Configurator<TService>::Remote>(invokerType, serviceId, TIMEOUT);
    }



};


}
}
