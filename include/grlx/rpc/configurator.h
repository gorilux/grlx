#pragma once


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
