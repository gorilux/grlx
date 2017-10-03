

#include "servicecontainer.h"


namespace grlx {

ServiceContainer::~ServiceContainer()
{
    objectMap.clear();

    if(parent)
        parent.reset();
}


ServiceContainerPtr ServiceContainerFactory::Create()
{
    return ServiceContainerPtr(new ServiceContainer());
}

ServiceContainerPtr ServiceContainerFactory::Create(const ServiceContainerPtr & parent)
{
    return ServiceContainerPtr(new ServiceContainer(parent));
}

}
