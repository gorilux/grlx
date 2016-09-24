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

#ifndef GRLX_ASYNC_ASYNCMANAGER_H
#define GRLX_ASYNC_ASYNCMANAGER_H

#include <memory>
#include <functional>
#include <sstream>
#include <limits>
#include <future>
#include <string>
#include <unordered_map>
#include <mutex>
#include <list>


namespace grlx {


template<typename IDType>
class AsyncManager;

namespace Details {


typedef long TypeId;


template<class T>
struct TypeInfo
{
    static TypeId id() { return reinterpret_cast<TypeId>( &inst() ); }
private:
    struct Impl{};
    static Impl const & inst() { static Impl sImpl; return sImpl; }
};

template<class T>
TypeId typeId()
{
    return TypeInfo<T>::id();
}

template<class T>
TypeId typeId(T)
{
    return TypeInfo<T>::id();
}



template<typename IDType>
struct AsyncManagerImpl;

}
template<typename IDType>
class AsyncOperationBase : public std::enable_shared_from_this<AsyncOperationBase<IDType>>
{
public:
    using Ptr = std::shared_ptr<AsyncOperationBase>;

    AsyncOperationBase(IDType&& id, AsyncManager<IDType>* asyncManager)
        : _id(id),
          _asyncManager(asyncManager){}

    const IDType& id() const
    {
        return _id;
    }

    virtual int typeId() const = 0;


protected:

    void completeOp()
    {
        _asyncManager->operationCompleted(this->shared_from_this());
    }

private:

    IDType _id;
    AsyncManager<IDType>* _asyncManager;



};

template<typename IDType, typename TFunc>
class AsyncOperation : public AsyncOperationBase<IDType>
{

public:
    using Ptr = std::shared_ptr<AsyncOperation>;


    AsyncOperation(IDType&& id, AsyncManager<IDType>* asyncManager,  TFunc&& func)
        : AsyncOperationBase<IDType>(std::move(id), asyncManager),
          _func(std::forward<TFunc>(func)){}


    AsyncOperation(const AsyncOperation&) = delete;
    AsyncOperation& operator=(const AsyncOperation&) = delete;


    template<typename ...TArgs>
    void complete(TArgs&&... result)
    {
        this->completeOp();

        this->_func(std::forward<TArgs>(result)...);
    }

    int typeId() const override
    {
        return Details::typeId<TFunc>();
    }

    AsyncOperation(IDType&& id)
        : AsyncOperationBase<IDType>(std::move(id)){}

private:
    friend class AsyncManager<IDType>;
    TFunc _func;

};

namespace Details {

template<typename IDType>
struct AsyncManagerImpl;

template<>
struct AsyncManagerImpl<std::string>
{
    class Apply
    {
    public:
        Apply(int turnoverValue = std::numeric_limits<int>::max(), const std::string& prefix = "", const std::string& suffix = "")
                : id ( 0 ), turnoverValue( turnoverValue ), prefix( prefix), suffix(suffix)
        { }
    protected:
        std::string createNextOpId()
        {
            if(++id == turnoverValue )
            {
                id = 0;
            }
            std::ostringstream ostr;
            ostr << prefix << id << suffix;
            return ostr.str();
        }
       private:
           int id;
           int turnoverValue;
           std::string prefix;
           std::string suffix;
    };

};

template<>
struct AsyncManagerImpl<int>
{
    class Apply
    {
    public:
        Apply(int turnoverValue = std::numeric_limits<int>::max())
                : id ( 0 ), turnoverValue( turnoverValue )
        { }
    protected:
        int createNextOpId()
        {
            if(++id == turnoverValue )
            {
                id = 0;
            }
            return id;
        }
       private:
           int id;
           int turnoverValue;

    };

};

}

template<typename IDType>
class AsyncManager : public Details::AsyncManagerImpl<IDType>::Apply
{

    using BaseType = typename Details::AsyncManagerImpl<IDType>::Apply;

    template<typename T>
    struct LambdaToFunc
    {
        using type = void;
    };

    template<typename Ret, typename Class, typename... Args>
    struct LambdaToFunc<Ret(Class::*)(Args...) const>
    {
        using type = std::function<Ret(Args...)>;
    };

public:

    template<typename R, typename ...A>
    using TFunc = std::function<R(A...)>;

    template<typename TResult, typename ...TArgs>
    typename AsyncOperation<IDType, TFunc<TResult, TArgs...> >::Ptr createOperation(TFunc<TResult, TArgs...>&& func)
    {        
        std::lock_guard<std::mutex> lock(_syncMtx);

        completedAsyncOps.clear();

        auto opId = BaseType::createNextOpId();
        auto asyncOp = std::make_shared<AsyncOperation<IDType, TFunc<TResult, TArgs...> > >(std::move(opId), this, std::forward<TFunc<TResult, TArgs...> >(func));

        outstandingAsyncOps[asyncOp->id()] = std::static_pointer_cast<AsyncOperationBase<IDType>>(asyncOp);

        return asyncOp;
    }

    template<typename TLambda>
    auto createOperation(TLambda&& func)
    {
        return createOperation(typename LambdaToFunc<decltype(&TLambda::operator())>::type(func));
    }
    template<typename TResult, typename ...TArgs>
    typename AsyncOperation<IDType,  TFunc<TResult, TArgs...> >::Ptr getOperation(IDType const& id) const
    {

        std::lock_guard<std::mutex> lock(_syncMtx);


        auto itr = outstandingAsyncOps.find(id);

        if(itr == outstandingAsyncOps.end())
            return typename AsyncOperation<IDType,  TFunc<TResult, TArgs...> >::Ptr();

        auto asyncOp = itr->second;

        if(asyncOp->typeId() != Details::typeId< TFunc<TResult, TArgs...>>())
        {
            return typename AsyncOperation<IDType,  TFunc<TResult, TArgs...> >::Ptr();
        }
        return std::static_pointer_cast< AsyncOperation<IDType,  TFunc<TResult, TArgs...> > >(asyncOp);
    }

private:
    void operationCompleted(typename AsyncOperationBase<IDType>::Ptr const& asyncOp)
    {
        std::lock_guard<std::mutex> lock(_syncMtx);

        completedAsyncOps.push_back(asyncOp);
    }

    friend class AsyncOperationBase<IDType>;

private:
    mutable std::mutex _syncMtx;
    std::unordered_map<IDType, typename AsyncOperationBase<IDType>::Ptr > outstandingAsyncOps;
    std::list<typename AsyncOperationBase<IDType>::Ptr > completedAsyncOps;

};


}

#endif // ASYNCMANAGER_H

