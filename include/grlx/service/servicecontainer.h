
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


#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>



namespace grlx {

template<typename T>
class IServiceCreatorCallback
{
public:
    virtual std::shared_ptr<T> operator()() = 0;
};



class ServiceContainer;
using ServiceContainerPtr = std::shared_ptr<ServiceContainer>;

class ServiceContainer
{
    ServiceContainer(ServiceContainer const&) = delete;  // disable copy
    ServiceContainer& operator=(const ServiceContainer&) = delete;
private:

    ServiceContainer(const ServiceContainerPtr& parent);

    ServiceContainer();

    friend class ServiceContainerFactory;

    class ServiceInfoBase
    {
    public:
        virtual ~ServiceInfoBase(){}
        virtual std::shared_ptr<void> ServiceInstance() = 0;


    };
    template<typename T>
    struct DelayedContructionServiceInfo : ServiceInfoBase
    {
        DelayedContructionServiceInfo( const std::function< std::shared_ptr<T>() > createCallback)
            :createCallback(createCallback)
        { }
        std::shared_ptr<void> ServiceInstance()
        {
            if(!instance)
            {
                instance = std::static_pointer_cast<void>( createCallback() );
            }
            return instance;
        }
    private:
        std::shared_ptr<void> instance;
        std::function< std::shared_ptr<T>() > createCallback;
    };

    template<typename T>
    struct InstanceHolderServiceInfo: ServiceInfoBase
    {
        InstanceHolderServiceInfo( T* instance)
            : instance( instance )
        {

        }
        InstanceHolderServiceInfo( const std::shared_ptr<T>& instance)
            : instance( std::static_pointer_cast<void> ( instance))
        {

        }
        std::shared_ptr<void> ServiceInstance()
        {
            return instance;
        }
    private:
        std::shared_ptr<void> instance;
    };

public:

    virtual ~ServiceContainer();

    static ServiceContainerPtr& globalInstance();

    template<typename T>
    void addService()
    {
        std::shared_ptr<ServiceInfoBase> serviceInfo( new InstanceHolderServiceInfo<T>( new T )  );

        auto id = std::type_index(typeid(T));

        this->addService(id, serviceInfo);

    }
    template<typename T>
    void addService( std::function< std::shared_ptr<T> () > serviceCreatorCallback )
    {
        std::shared_ptr<ServiceInfoBase> serviceInfo( new DelayedContructionServiceInfo<T>( serviceCreatorCallback )  );

        auto id = std::type_index(typeid(T));

        this->addService(id, serviceInfo);
    }
    template<typename T, typename U>
    void addService(const std::shared_ptr<U>& serviceInstance)
    {
        std::shared_ptr<ServiceInfoBase> serviceInfo( new InstanceHolderServiceInfo<T>( serviceInstance )  );
        auto id = std::type_index(typeid(T));

        this->addService(id, serviceInfo);
    }
    template<typename T, typename U>
    void addService(U* serviceInstance)
    {
        this->addService<T>(std::shared_ptr<T>(serviceInstance));
    }

    template<typename T>
    std::shared_ptr<T> getService()
    {
        auto id = std::type_index(typeid(T));
        TypeInfoMap::iterator itr = objectMap.find(id);

        if(itr == objectMap.end()){
            if(!parent)
                return std::shared_ptr<T>();

            return parent->getService<T>();
        }

        return std::static_pointer_cast<T>( itr->second->ServiceInstance() );
    }
    template<typename T>
    std::shared_ptr<T> get()
    {
        return getService<T>();
    }

    template<typename T>
    void removeService()
    {
        auto id = std::type_index(typeid(T));
        objectMap.erase(id);
    }
    template<typename T>
    bool hasService()
    {
        return (objectMap.find(std::type_index(typeid(T))) != objectMap.end());
    }



private:

    void addService(const std::type_index& id, const std::shared_ptr<ServiceInfoBase>& serviceInfoBase)
    {
        if(objectMap.find(id) != objectMap.end())
        {
            //throw grlx::Exception("Service already exists", 0);
            //
        }
        objectMap[id] = serviceInfoBase;
    }

private:

    using TypeInfoMap = std::unordered_map<std::type_index, std::shared_ptr<ServiceInfoBase> > ;
    ServiceContainerPtr parent;

    mutable TypeInfoMap objectMap;

};



class ServiceContainerFactory
{
public:

    static ServiceContainerPtr create();

    static ServiceContainerPtr create(const ServiceContainerPtr& parent);
};


}
