/*
 *    Copyright (c) 2011 David Salvador Pinheiro
 *
 *    http://www.gorilux.org
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

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
