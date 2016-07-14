

#include "servicecontainer.h"


namespace grlx {

ServiceContainerPtr ServiceContainerFactory::Create()
{
    return ServiceContainerPtr(new ServiceContainer());
}

ServiceContainerPtr ServiceContainerFactory::Create(const ServiceContainerPtr & parent)
{
    return ServiceContainerPtr(new ServiceContainer(parent));
}

}
