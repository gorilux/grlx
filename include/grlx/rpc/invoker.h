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
#include <tuple>

#include <grlx/async/asyncmanager.h>
#include <grlx/tmpl/callfunc.h>
#include <grlx/async/threadpool.h>

#include "utility.h"
#include "types.h"
#include "message.h"



namespace grlx
{
namespace rpc
{

template<typename EncoderType, typename TransportType >
class Invoker
{

public:
    using Type = Invoker;

    template<typename ...TArgs>
    Invoker(TArgs&&... args)
        : transportLayer(std::forward<TArgs>(args)...)
    {
       //transportLayer.registerMsgHandler(std::bind(&Invoker::handleResp, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
    ~Invoker(){}


    template<typename ...TArgs>
    void start(TArgs&&... args)
    {
        transportLayer.start(std::forward<TArgs>(args)...);
    }

    void stop()
    {
        transportLayer.stop();
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
//        transportLayer.sendMsg(*streamBuff, [streamBuff, promise](auto const& ec)
//        {

//        });

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
//        transportLayer.sendMsg(*streamBuff, [streamBuff, promise](auto const& ec)
//        {

//        });
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
//        transportLayer.sendMsg(*streamBuff, [streamBuff](auto const& ec)
//        {

//        });


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
//        transportLayer.sendMsg(*streamBuff, [streamBuff](auto const& ec)
//        {

//        });


    }

    template<typename... TArgs>
    void notify(std::string&& procName, TArgs&&... args )
    {

        Notification<TArgs...> notification(std::forward<std::string>(procName), std::forward<TArgs>(args)...);


        auto streamBuff = transportLayer.nextBuffer();
        std::ostream os(streamBuff.get());
        EncoderType::encode(notification, os);
//        transportLayer.sendMsg(*streamBuff, [streamBuff](auto const& ec)
//        {

//        });
    }


private:

    void handleResp(const char* msg, size_t size, TokenType const& userToken)
    {
        std::string str(msg, size);
        std::stringstream is(str);
        EncoderType::decodeResp(is, *this);
    }

private:
    friend EncoderType;


    void reply(int id, typename EncoderType::ResultType& result)
    {
        auto asyncOp = asyncManager.getOperation<void, typename EncoderType::ResultType &>(id);

        if(asyncOp)
        {
            asyncOp->complete(result);
        }
    }

private:    
    TransportType  transportLayer;
    AsyncManager<int> asyncManager;
    async::ThreadPool<> threadPool;

};


}
}
