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


#ifndef GRLX_SERVICECONTAINER_H
#define	GRLX_SERVICECONTAINER_H



#include <string>
#include <functional>
#include <memory>
#include <map>

#include <grlx/tmpl/typeid.h>



namespace grlx {

template<typename T>
class IServiceCreatorCallback
{
public:
    virtual std::shared_ptr<T> operator()() = 0;
};



class ServiceContainer;
typedef std::shared_ptr<ServiceContainer> ServiceContainerPtr;

class ServiceContainer
{
    ServiceContainer(ServiceContainer const&) = delete;  // disable copy
    ServiceContainer& operator=(const ServiceContainer&) = delete;
private:

    ServiceContainer(const ServiceContainerPtr& parent)
        : parent(parent)
    {}

    ServiceContainer()
    {}




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

    virtual ~ServiceContainer()
    {
        objectMap.clear();

        if(parent)
            parent.reset();
    }

    template<typename T>
    void AddService()
    {
        std::shared_ptr<ServiceInfoBase> serviceInfo( new InstanceHolderServiceInfo<T>( new T )  );

        this->AddService(typeId<T>(), serviceInfo);

    }
    template<typename T>
    void AddService( std::function< std::shared_ptr<T> () > serviceCreatorCallback )
    {
        std::shared_ptr<ServiceInfoBase> serviceInfo( new DelayedContructionServiceInfo<T>( serviceCreatorCallback )  );

        this->AddService(typeId<T>(), serviceInfo);
    }
    template<typename T, typename U>
    void AddService(const std::shared_ptr<U>& serviceInstance)
    {
        std::shared_ptr<ServiceInfoBase> serviceInfo( new InstanceHolderServiceInfo<T>( serviceInstance )  );

        this->AddService(typeId<T>(), serviceInfo);
    }
    template<typename T, typename U>
    void AddService(U* serviceInstance)
    {
        this->AddService<T>(std::shared_ptr<T>(serviceInstance));
    }

    template<typename T>
    std::shared_ptr<T> GetService()
    {
        TypeId id = typeId<T>();
        TypeInfoMap::iterator itr = objectMap.find(id);

        if(itr == objectMap.end()){
            if(!parent)
                return std::shared_ptr<T>();

            return parent->GetService<T>();
        }

        return std::static_pointer_cast<T>( itr->second->ServiceInstance() );
    }
    template<typename T>
    std::shared_ptr<T> Get()
    {
        return GetService<T>();
    }

    template<typename T>
    void RemoveService()
    {
        TypeId id = typeId<T>();
        objectMap.erase(id);
    }
    template<typename T>
    bool HasService()
    {
        return (objectMap.find(typeId<T>()) != objectMap.end());
    }



private:

    void AddService( grlx::TypeId id, const std::shared_ptr<ServiceInfoBase>& serviceInfoBase)
    {
        if(objectMap.find(id) != objectMap.end())
        {
            //throw grlx::Exception("Service already exists", 0);
        }
        objectMap[id] = serviceInfoBase;
    }

private:

    typedef std::map<grlx::TypeId, std::shared_ptr<ServiceInfoBase> > TypeInfoMap;
    ServiceContainerPtr parent;

    mutable TypeInfoMap objectMap;

};



class ServiceContainerFactory
{
public:
    
    static ServiceContainerPtr Create();
    
    static ServiceContainerPtr Create(const ServiceContainerPtr& parent);
};


}

#endif	/* SERVICECONTAINER_H */

