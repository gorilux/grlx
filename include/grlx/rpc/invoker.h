#pragma once


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

    template<typename R, typename... TArgs>
    auto execute(std::string&& procName, TArgs&&... args )
            -> typename std::enable_if<!std::is_void<R>::value, std::future<R> >::type
    {
        auto promise = std::make_shared<std::promise<R>>();

        auto asyncOp = asyncManager.createOperation(
            [promise,this](typename EncoderType::ResultType& result)
            {
                R res;
                EncoderType::decodeType(result, res);
                promise->set_value(res);
            });

        Request<R, TArgs...> request(std::forward<std::string>(procName),
                                  asyncOp->id(),
                                  std::forward<TArgs>(args)...);

        EncoderType::encode(request, [&](const char* data, size_t size)
        {
            this->send(data,size,nullptr);
        });

        return promise->get_future();

    }

    template<typename R, typename... TArgs>
    auto execute(std::string&& procName, TArgs&&... args )
            -> typename std::enable_if<std::is_void<R>::value, std::future<void> >::type
    {
        auto promise = std::make_shared<std::promise<void>>();

        auto asyncOp = asyncManager.createOperation(
            [promise,this](typename EncoderType::ResultType& result)
            {
                promise->set_value();
            });

        Request<R, TArgs...> request(std::forward<std::string>(procName),
                                  asyncOp->id(),
                                  std::forward<TArgs>(args)...);

        EncoderType::encode(request, [&](const char* data, size_t size)
        {
            this->send(data,size,nullptr);
        });

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

        EncoderType::encode(request, [&](const char* data, size_t size)
        {
            this->send(data,size, nullptr);
        });


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

        EncoderType::encode(request, [&](const char* data, size_t size)
        {
            this->send(data,size, nullptr);
        });


    }

    template<typename... TArgs>
    void notify(std::string&& procName, TArgs&&... args )
    {

        Notification<TArgs...> notification(std::forward<std::string>(procName), std::forward<TArgs>(args)...);

        EncoderType::encode(notification, [&](const char* data, size_t size)
        {
            this->send(data,size, nullptr);
        });
    }

protected:
    virtual void send(const char* msg, size_t len, TokenType const& userToken) = 0;

    void handleResp(const char* msg, size_t size, TokenType const& userToken)
    {
        EncoderType::decodeResp(msg, size, static_cast<SelfType&>(*this));
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
    AsyncManager<int> asyncManager;
    async::ThreadPool<> threadPool;

};


}
}
