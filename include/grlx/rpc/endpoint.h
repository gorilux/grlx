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


#include <type_traits>
#include <utility>
#include <cstdint>
#include <future>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <grlx/async/asyncmanager.h>
#include <grlx/rpc/invoker.h>

#include "utility.h"
#include "types.h"
#include "message.h"

namespace grlx {

namespace rpc
{

template<typename EncoderType, typename TransportLayerType>
class Endpoint
{

private:

    struct DispatcherBase
    {
        DispatcherBase(Endpoint* self) : _self(self){}
        virtual ~DispatcherBase(){}

        virtual void operator()(const int* id, typename EncoderType::ParamsType&  params, TokenType const& userToken) = 0;

        virtual std::type_index typeIndex() const = 0;

        Endpoint *_self;
    };

    template<typename TSignature>
    struct Dispatcher : DispatcherBase
    {

        Dispatcher(Endpoint* self)
            : DispatcherBase(self) {}

        std::function<TSignature> proc;

        template<typename TParams, typename R, typename ...ArgsT>
        R dispatch(TParams& params, std::function<R(ArgsT...)>& func)
        {

            std::tuple< typename std::decay<ArgsT>::type ... > args;

            EncoderType::decodeType(params, args);

            //return grlx::callFunc(func, args);
            return std::apply(func, args);
        }

        template<typename TParams, typename ...ArgsT>
        void dispatchAndReply(int id, TParams& params, std::function<void(ArgsT...)>& func, TokenType const& userToken)
        {

            Reply<void> reply;
            reply.id = id;

            dispatch(params, func);

            auto streamBuff = this->_self->transportLayer.nextBuffer();

            std::ostream os(streamBuff.get());

            EncoderType::encode(reply, os);

//            this->_self->transportLayer_.sendMsg(streamBuff, [streamBuff](auto ec){

//            });
            // TODO::Send ostr

        }

        template<typename TParams, typename R, typename ...ArgsT>
        void dispatchAndReply(int id, TParams& params, std::function<R(ArgsT...)>& func, TokenType const& userToken)
        {
            Reply<R> reply;
            reply.id = id;

            reply.result = dispatch(params, func);

            std::ostringstream ostr;
            EncoderType::encode(reply, ostr);

            // TODO::Send ostr


        }

        std::type_index typeIndex() const override
        {
            return std::type_index(typeid(TSignature));
        }

        void operator ()(const int* id, typename EncoderType::ParamsType& params, TokenType const& userToken) override
        {
            if( id == nullptr)
            {
                dispatch(params, proc);
            }
            else
            {
                dispatchAndReply(*id, params, proc, userToken);
            }
        }
    };

    using HandlerPtr = std::shared_ptr< DispatcherBase >;

public:


    using DataHandler = std::function<int(const char*, int)>;
    using Type = Endpoint< EncoderType, TransportLayerType>;

    template<typename ...TArgs>
    Endpoint( TArgs&&... args)
        : transportLayer(std::forward<TArgs>(args)...)
    {

    }
    virtual ~Endpoint(){}


    template<typename R, typename C, typename ...ArgsT>
    void attach(std::string&& name, C* objPtr, R(C::*memFunc)(ArgsT...) const)
    {

        using DispatcherType = Dispatcher<R(ArgsT...)>;

        auto dispatcher = std::make_shared< DispatcherType >( this );

        dispatcher->proc = [&, objPtr, memFunc](ArgsT&&... args) -> R
        {
           return (objPtr->*memFunc)(args...);
        };


        this->attach(std::forward<std::string>(name), std::move(dispatcher));
    }

    template<typename R, typename C, typename ...ArgsT>
    void attach(std::string&& name, C* objPtr,  R(C::*memFunc)(ArgsT...))
    {

        using DispatcherType = Dispatcher<R(ArgsT...)>;

        auto dispatcher = std::make_shared< DispatcherType >( this );

        dispatcher->proc = [&, objPtr, memFunc](ArgsT&&... args) -> R
        {
            return (objPtr->*memFunc)(args...);
        };


        this->attach(std::forward<std::string>(name), std::move(dispatcher));
    }

    template<typename TSignature>
    void attach(std::string&& name, std::function<TSignature>&& func)
    {

        using DispatcherType = Dispatcher<TSignature>;

        auto dispatcher = std::make_shared< DispatcherType >( this );

        std::swap(dispatcher->proc, func);

        this->attach(std::forward<std::string>(name), std::move(dispatcher));
    }

    template<typename F>
    void attach(std::string&& name, F&& func)
    {
        attach(std::move(name), typename DeduceLambdaSignature<decltype(&F::operator())>::type(func));
    }

    template<typename R, typename... TArgs>
    auto execute(std::string&& procName, TArgs&&... args )
            -> typename std::enable_if<!std::is_void<R>::value, std::future<R> >::type
    {
        auto promise = std::make_shared<std::promise<R>>();

        auto asyncOp = asyncManager.createOperation(
            [promise](typename EncoderType::ResultType& result)
            {
                R res;
                EncoderType::decodeType(result, res);
                promise->set_value(res);
            });

        Request<R, TArgs...> request(std::forward<std::string>(procName),
                                  asyncOp->id(),
                                  std::forward<TArgs>(args)...);

        auto streamBuff = transportLayer.nextBuffer();
        std::ostream os(streamBuff.get());
        EncoderType::encode(request, os);
        transportLayer.sendMsg(std::move(streamBuff));

        return promise->get_future();

    }

    template<typename R, typename... TArgs>
    auto execute(std::string&& procName, TArgs&&... args )
            -> typename std::enable_if<std::is_void<R>::value, std::future<void> >::type
    {
        auto promise = std::make_shared<std::promise<void>>();

        auto asyncOp = asyncManager.createOperation(
            [promise](typename EncoderType::ResultType& result)
            {
                promise->set_value();
            });

        Request<R, TArgs...> request(std::forward<std::string>(procName),
                                  asyncOp->id(),
                                  std::forward<TArgs>(args)...);

        auto streamBuff = transportLayer.nextBuffer();
        std::ostream os(streamBuff.get());
        EncoderType::encode(request, os);
        transportLayer.sendMsg(std::move(streamBuff));

        return promise->get_future();

    }

    template<typename R, typename F, typename ...TArgs>
    typename std::enable_if<!std::is_void<R>::value>::type
    invokeAsync(F&& callback, std::string&& procName, TArgs&&... args)
    {

        auto asyncOp = asyncManager.createOperation(
            [f = std::move(callback), this ](typename EncoderType::ResultType& result)
            {
                R res;
                EncoderType::decodeType(result, res);
                threadPool.schedule(std::move(f), std::move(res) );
            });

        Request<R, TArgs...> request(std::forward<std::string>(procName),
                                  asyncOp->id(),
                                  std::forward<TArgs>(args)...);

        auto streamBuff = transportLayer.nextBuffer();
        std::ostream os(streamBuff.get());
        EncoderType::encode(request, os);
        transportLayer.sendMsg(std::move(streamBuff));


    }

    template<typename R, typename F, typename ...TArgs>
    typename std::enable_if<std::is_void<R>::value>::type
    invokeAsync(F&& callback, std::string&& procName, TArgs&&... args)
    {

        auto asyncOp = asyncManager.createOperation(
            [f = std::move(callback), this ](typename EncoderType::ResultType const& result)
            {
                threadPool.schedule(std::move(f));
            });

        Request<R, TArgs...> request(std::forward<std::string>(procName),
                                  asyncOp->id(),
                                  std::forward<TArgs>(args)...);

        auto streamBuff = transportLayer.nextBuffer();
        std::ostream os(streamBuff.get());
        EncoderType::encode(request, os);
        transportLayer.sendMsg(std::move(streamBuff));

    }

    template<typename... TArgs>
    void notify(std::string&& procName, TArgs&&... args )
    {

        Notification<TArgs...> notification(std::forward<std::string>(procName), std::forward<TArgs>(args)...);


        auto streamBuff = transportLayer.nextBuffer();
        std::ostream os(streamBuff.get());
        EncoderType::encode(notification, os);
        transportLayer.sendMsg(std::move(streamBuff));
    }

private:

    void handleReq(const char* msg, int size, TokenType const& userToken)
    {
        std::string str(msg, size);
        std::stringstream is(str);
        EncoderType::decodeReq(is, *this, userToken);
    }

    void handleResp(const char* msg, size_t size, TokenType const& userToken)
    {
        std::string str(msg, size);
        std::stringstream is(str);
        EncoderType::decodeResp(is, *this);
    }


private:

    friend EncoderType;


    template<typename Dispatcher>
    void attach(std::string&& name, std::shared_ptr<Dispatcher>&& handler)
    {
        std::string signature = handler->typeIndex().name();
        dispatchTable.emplace(std::forward<std::string>(name), handler);
    }

    template<typename TParams>
    void exec(const std::string& method, int id, TParams& params, TokenType const& userToken)
    {
        auto itr = dispatchTable.find(method);
        if(itr != dispatchTable.end())
        {
            auto& func= *(itr->second);
            func(&id, params, userToken);
        }
    }

    template<typename TParams>
    void exec(const std::string& method, TParams& params, TokenType const& userToken)
    {
        auto itr = dispatchTable.find(method);
        if(itr != dispatchTable.end())
        {
            auto& func = *(itr->second);
            func(nullptr, params, userToken);
        }
    }


    void error(const std::string& method, int id)
    {
        std::cerr << "ERROR:" << method << " " << id << std::endl;
    }

    void error(const std::string& method)
    {
        std::cerr << "ERROR:" << method << std::endl;

    }




private:
    friend EncoderType;

    TransportLayerType transportLayer;
    std::unordered_map<std::string, HandlerPtr> dispatchTable;
    AsyncManager<int> asyncManager;
    async::ThreadPool<> threadPool;
};


}
}
