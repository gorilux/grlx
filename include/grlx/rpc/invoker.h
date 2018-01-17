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

#ifndef GRLX_RPC_INVOKER_H
#define GRLX_RPC_INVOKER_H


#include <type_traits>
#include <utility>
#include <cstdint>
#include <future>
#include <functional>
#include <memory>
#include <unordered_map>

#include <grlx/async/asyncmanager.h>
#include <grlx/tmpl/callfunc.h>

#include "connection.h"
#include "utility.h"
#include "types.h"
#include "message.h"



namespace grlx
{
namespace rpc
{

template<typename EncoderType, typename BaseType = Details::DummyBaseClass, typename DerivedType = Details::None >
class Invoker: public BaseType
{

    using SelfType = typename std::conditional<
        std::is_same<DerivedType, Details::None>::value, Invoker, DerivedType
        >::type;

public:


    using Type = Invoker;

    template<typename ...TArgs>
    Invoker( TArgs&&... args)
        : BaseType(std::forward<TArgs>(args)...)
    {

    }
    virtual ~Invoker(){}


    // template<typename R, typename... TArgs>
    // std::future<R> execute(std::string&& procName, TArgs&&... args )
    // {
    //     auto promise = std::make_shared<std::promise<R>>();

    //     auto asyncOp = asyncManager.createOperation(
    //         [promise](typename EncoderType::ResultType const& result)
    //         {
    //             R res;
    //             EncoderType::decodeType(result, res);
    //             promise->set_value(res);
    //         });

    //     Request<TArgs...> request(std::forward<std::string>(procName),
    //                               asyncOp->id(),
    //                               std::forward<TArgs>(args)...);

    //     EncoderType::encode(request, [&](const char* data, size_t size)
    //     {
    //         this->send(data,size);
    //     });

    //     return promise->get_future();

    // }

    template<typename R, typename... TArgs>
    auto execute(std::string&& procName, TArgs&&... args )
            -> typename std::enable_if<!std::is_void<R>::value, std::future<R> >::type
    {
        auto promise = std::make_shared<std::promise<R>>();

        auto asyncOp = asyncManager.createOperation(
            [promise](typename EncoderType::ResultType const& result)
            {
                R res;
                EncoderType::decodeType(result, res);
                promise->set_value(res);
            });

        Request<TArgs...> request(std::forward<std::string>(procName),
                                  asyncOp->id(),
                                  std::forward<TArgs>(args)...);

        EncoderType::encode(request, [&](const char* data, size_t size)
        {
            this->send(data,size);
        });

        return promise->get_future();

    }

    template<typename R, typename... TArgs>
    auto execute(std::string&& procName, TArgs&&... args )
            -> typename std::enable_if<std::is_void<R>::value, std::future<void> >::type
    {
        auto promise = std::make_shared<std::promise<void>>();

        auto asyncOp = asyncManager.createOperation(
            [promise](typename EncoderType::ResultType const& result)
            {
                promise->set_value();
            });

        Request<TArgs...> request(std::forward<std::string>(procName),
                                  asyncOp->id(),
                                  std::forward<TArgs>(args)...);

        EncoderType::encode(request, [&](const char* data, size_t size)
        {
            this->send(data,size);
        });

        return promise->get_future();

    }

    template<typename R, typename F, typename ...TArgs>
    typename std::enable_if<!std::is_void<R>::value>::type
    invokeAsync(F&& callback, std::string&& procName, TArgs&&... args)
    {

        auto asyncOp = asyncManager.createOperation(
            [f = std::move(callback) ](typename EncoderType::ResultType const& result)
            {
                R res;
                EncoderType::decodeType(result, res);
                f( res );
            });

        Request<TArgs...> request(std::forward<std::string>(procName),
                                  asyncOp->id(),
                                  std::forward<TArgs>(args)...);

        EncoderType::encode(request, [&](const char* data, size_t size)
        {
            this->send(data,size);
        });


    }

    template<typename R, typename F, typename ...TArgs>
    typename std::enable_if<std::is_void<R>::value>::type
    invokeAsync(F&& callback, std::string&& procName, TArgs&&... args)
    {

        auto asyncOp = asyncManager.createOperation(
            [f = std::move(callback) ](typename EncoderType::ResultType const& result)
            {
                f();
            });

        Request<TArgs...> request(std::forward<std::string>(procName),
                                  asyncOp->id(),
                                  std::forward<TArgs>(args)...);

        EncoderType::encode(request, [&](const char* data, size_t size)
        {
            this->send(data,size);
        });


    }

    template<typename... TArgs>
    void notify(std::string&& procName, TArgs&&... args )
    {

        Notification<TArgs...> notification(std::forward<std::string>(procName), std::forward<TArgs>(args)...);

        EncoderType::encode(notification, [&](const char* data, size_t size)
        {
            this->send(data,size);
        });
    }

protected:
    virtual void send(const char* msg, size_t len) = 0;

    void handleResp(const char* msg, size_t size)
    {
        EncoderType::decodeResp(msg, size, static_cast<SelfType&>(*this));
    }

private:
    friend EncoderType;


    void reply(int id, typename EncoderType::ResultType const& result)
    {
        auto asyncOp = asyncManager.getOperation<void, typename EncoderType::ResultType const &>(id);

        if(asyncOp)
        {
            asyncOp->complete(result);
        }
    }

private:
    AsyncManager<int> asyncManager;

};


}
}



#endif // GRLX_RPC_INVOKER_H
