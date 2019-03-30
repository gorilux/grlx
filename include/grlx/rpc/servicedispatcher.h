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
#ifndef GRLX_RPC_SERVICEPROVIDER_H
#define GRLX_RPC_SERVICEPROVIDER_H

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

template<typename EncoderType, typename BaseType = Details::DummyBaseClass>
class ServiceDispatcher : public BaseType
{

private:

//    template<typename T>
//    struct DeduceLambdaSignature
//    {
//        using type = void;
//    };

//    template<typename Ret, typename Class, typename... Args>
//    struct DeduceLambdaSignature<Ret(Class::*)(Args...) const>
//    {
//        using type = std::function<Ret(Args...)>;
//    };

    struct HandlerBase
    {
        HandlerBase(ServiceDispatcher* self) : _self(self){}

        virtual void operator()(const int* id, typename EncoderType::ParamsType&  params, TokenType const& userToken) = 0;

        virtual std::type_index typeIndex() const = 0;

        ServiceDispatcher *_self;
    };

    template<typename TSignature>
    struct Handler : HandlerBase
    {

        Handler(ServiceDispatcher* self)
            : HandlerBase(self) {}

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

            EncoderType::encode(reply, [this, userToken](const char* data, size_t size)
            {
                this->_self->send(data,size, userToken);
            });

        }

        template<typename TParams, typename R, typename ...ArgsT>
        void dispatchAndReply(int id, TParams& params, std::function<R(ArgsT...)>& func, TokenType const& userToken)
        {
            Reply<R> reply;
            reply.id = id;

            reply.result = dispatch(params, func);

            EncoderType::encode(reply, [this, userToken](const char* data, size_t size)
            {
                this->_self->send(data,size, userToken);
            });
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

    using HandlerPtr = std::shared_ptr< HandlerBase >;

public:


    using DataHandler = std::function<int(const char*, int)>;
    using Type = ServiceDispatcher< EncoderType, BaseType>;

    template<typename ...TArgs>
    ServiceDispatcher( TArgs&&... args)
        : BaseType(std::forward<TArgs>(args)...)
    {

    }
    virtual ~ServiceDispatcher(){}


    template<typename R, typename C, typename ...ArgsT>
    void attach(std::string&& name, C* objPtr, R(C::*memFunc)(ArgsT...) const)
    {

        using HandlerType = Handler<R(ArgsT...)>;

        auto handler = std::make_shared< HandlerType >( this );

        handler->proc = [&, objPtr, memFunc](ArgsT&&... args) -> R
        {
           return (objPtr->*memFunc)(args...);
        };


        this->attach(std::forward<std::string>(name), std::move(handler));
    }

    template<typename R, typename C, typename ...ArgsT>
    void attach(std::string&& name, C* objPtr,  R(C::*memFunc)(ArgsT...))
    {

        using HandlerType = Handler<R(ArgsT...)>;

        auto handler = std::make_shared< HandlerType >( this );

        handler->proc = [&, objPtr, memFunc](ArgsT&&... args) -> R
        {
            return (objPtr->*memFunc)(args...);
        };


        this->attach(std::forward<std::string>(name), std::move(handler));
    }

    template<typename TSignature>
    void attach(std::string&& name, std::function<TSignature>&& func)
    {

        using HandlerType = Handler<TSignature>;


        auto handler = std::make_shared< HandlerType >( this );

        std::swap(handler->proc, func);                

        this->attach(std::forward<std::string>(name), std::move(handler));
    }

    template<typename F>
    void attach(std::string&& name, F&& func)
    {
        attach(std::move(name), typename DeduceLambdaSignature<decltype(&F::operator())>::type(func));
    }


protected:
    virtual void send(const char* data, size_t size, TokenType const& userToken) = 0;

    void handleMessage(const char* msg, int size, TokenType const& userToken)
    {
        EncoderType::decodeReq(msg, size, *this, userToken);
    }


private:

    friend EncoderType;


    template<typename Handler>
    void attach(std::string&& name, std::shared_ptr<Handler>&& handler)
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

    std::unordered_map<std::string, HandlerPtr> dispatchTable;
};


}
}

#endif // GRLX_RPC_SERVICEPROVIDER_H
