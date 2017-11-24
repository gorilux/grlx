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
#include <grlx/async/asyncmanager.h>
#include <grlx/tmpl/callfunc.h>
#include <grlx/rpc/invoker.h>

#include "connection.h"
#include "utility.h"
#include "types.h"
#include "message.h"

namespace grlx {

namespace rpc
{

template<typename EncoderType, typename BaseType = Details::DummyBaseClass>
class ServiceProvider : public BaseType //public Invoker<EncoderType, BaseType, ServiceProvider<EncoderType, BaseType> >
{

public:

    class Session;

    using SessionPtr = std::shared_ptr<Session>;

private:

    struct HandlerBase
    {
        HandlerBase(ServiceProvider* self) : _self(self){}

        virtual void operator()( SessionPtr session, const int* id, typename EncoderType::ParamsType const&  params) = 0;

        ServiceProvider *_self;
    };

    template<typename TSignature>
    struct Handler : HandlerBase
    {

        Handler(ServiceProvider* self)
            : HandlerBase(self) {}

        std::function<TSignature> proc;

        template<typename TParams, typename R, typename ...ArgsT>
        R dispatch(SessionPtr session, TParams& params, std::function<R(ArgsT...)>& func)
        {

            std::tuple< typename std::decay<ArgsT>::type ... > args;

            EncoderType::decodeType(params, args);

            return grlx::callFunc(func, args);
        }

        template<typename TParams, typename ...ArgsT>
        void dispatchAndReply(SessionPtr session, int id, TParams& params, std::function<void(ArgsT...)>& func)
        {

            Reply<void> reply;
            reply.id = id;

            dispatch(session, params, func);

            EncoderType::encode(reply, *session);

        }

        template<typename TParams, typename R, typename ...ArgsT>
        void dispatchAndReply(SessionPtr session, int id, TParams& params, std::function<R(ArgsT...)>& func)
        {
            Reply<R> reply;
            reply.id = id;

            reply.result = dispatch(session, params, func);

            EncoderType::encode(reply, *session);
        }

        void operator ()(SessionPtr session, const int* id, typename EncoderType::ParamsType const& params)
        {
            if( id == nullptr)
            {
                dispatch(session, params, proc);
            }
            else
            {
                dispatchAndReply(session, *id, params, proc);
            }
        }
    };

    using HandlerPtr = std::shared_ptr< HandlerBase >;

public:

    class Session : public Invoker<  EncoderType, std::enable_shared_from_this<Session>, Session>
    {

        friend ServiceProvider;
        friend EncoderType;
    public:
        Session(ServiceProvider* serviceProvider)
            : serviceProvider(serviceProvider)
        {
        }
    protected:


        template<typename TParams>
        void exec(const std::string& method, int id, TParams const& params)
        {
            serviceProvider->exec( this->shared_from_this(), method, id, params);

        }

        template<typename TParams>
        void exec(const std::string& method, TParams const& params)
        {
            serviceProvider->exec( this->shared_from_this(), method, params);
        }

        void error(const std::string& method, int id)
        {

        }

        void error(const std::string& method)
        {

        }

        int handleMessage(const char* msg, int size)
        {
            EncoderType::decode(msg, size, *this);
            return 0;
        }

        void send(const char* msg, int size)
        {
            sendMsgDelegate(msg,size);
        }


        ServiceProvider* serviceProvider;
        MsgHandler sendMsgDelegate;

    };

    using DataHandler = std::function<int(const char*, int)>;
    using Type = ServiceProvider< EncoderType, BaseType>;

    template<typename ...TArgs>
    ServiceProvider( TArgs&&... args)
        : BaseType(std::forward<TArgs>(args)...)//Invoker<EncoderType, BaseType, ServiceProvider<EncoderType, BaseType> >(std::forward<TArgs>(args)...)
    {

    }
    virtual ~ServiceProvider(){}


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

    template<typename TConnectionType>
    SessionPtr createSession( TConnectionType* newConnection)
    {
        auto session = std::make_shared<Session>( this );

        //session.bind( newConnection );
        session->sendMsgDelegate = std::bind(&TConnectionType::sendMsg, newConnection, std::placeholders::_1, std::placeholders::_2);
        newConnection->setMsgHandler(std::bind(&Session::handleMessage, session.get(), std::placeholders::_1, std::placeholders::_2));


        outstandingSessions.insert( session );
        return session;
    }

    void destroySession( SessionPtr session )
    {
        outstandingSessions.erase( session );
    }

    template<typename TConnectionType>
    void bind(TConnectionType* connection)
    {
        currentSession = createSession(connection);
    }

    template<typename R, typename... TArgs>
    std::future<R> execute(std::string&& procName, TArgs&&... args )
    {
        return currentSession->template execute<R>(std::forward<std::string>(procName), std::forward<TArgs>(args)...);
    }

    template<typename R, typename F, typename ...TArgs>
    void invokeAsync(F&& callback, std::string&& procName, TArgs&&... args)
    {
        currentSession->template invokeAsync<R>(std::forward<F>(callback), std::forward<std::string>(procName), std::forward<TArgs>(args)...);
    }

    template<typename... TArgs>
    void notify(std::string&& procName, TArgs&&... args )
    {
        currentSession->notify(std::forward<std::string>(procName), std::forward<TArgs>(args)...);

    }



private:

    friend EncoderType;


    // int handleMessage(const char* msg, int size)
    // {
    //     EncoderType::decodeMsg(msg, size, *this);
    //     return 0;
    // }

    // void send(const char* msg, int size)
    // {
    //     sendMsgDelegate(msg, size);
    // }

    template<typename Handler>
    void attach(std::string&& name, std::shared_ptr<Handler>&& handler)
    {
        dispatchTable.emplace(std::forward<std::string>(name), handler);
    }

    template<typename TParams>
    void exec(SessionPtr session, const std::string& method, int id, TParams const& params)
    {
        auto itr = dispatchTable.find(method);
        if(itr != dispatchTable.end())
        {
            auto& func= *(itr->second);
            func(session, &id, params);
        }
    }

    template<typename TParams>
    void exec(SessionPtr session, const std::string& method, TParams const& params)
    {
        auto itr = dispatchTable.find(method);
        if(itr != dispatchTable.end())
        {
            auto& func = *(itr->second);
            func(session, nullptr, params);
        }
    }


private:
    friend EncoderType;

    std::unordered_map<std::string, HandlerPtr> dispatchTable;
    std::unordered_set<SessionPtr> outstandingSessions;
    SessionPtr currentSession;
};


}
}

#endif // GRLX_RPC_SERVICEPROVIDER_H
