

#include "servicecontainer.h"


namespace grlx {

ServiceContainer::~ServiceContainer()
{
    objectMap.clear();

    if(parent)
        parent.reset();
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
