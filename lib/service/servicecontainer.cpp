

#include "grlx/service/servicecontainer.h"


namespace grlx {

ServiceContainer::ServiceContainer(const ServiceContainerPtr &parent)
    : parent(parent)
{}

ServiceContainer::ServiceContainer()
{}

ServiceContainer::~ServiceContainer()
{
    objectMap.clear();

    if(parent)
        parent.reset();
}

ServiceContainerPtr& ServiceContainer::globalInstance()
{
    static auto instance = ServiceContainerPtr(new ServiceContainer());
    return instance;
}


ServiceContainerPtr ServiceContainerFactory::create()
{
    return ServiceContainerPtr(new ServiceContainer());
}

ServiceContainerPtr ServiceContainerFactory::create(const ServiceContainerPtr & parent)
{
    return ServiceContainerPtr(new ServiceContainer(parent));
}

}
