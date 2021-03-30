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
#include <grlx/rpc/types.h>
#include <grlx/rpc/message.h>
#include <grlx/rpc/namespaces.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/streambuf.hpp>

#include "utility.h"

namespace grlx {

namespace rpc
{


template<typename EncoderType>
class ServiceHost
{

private:

    struct ActionBase
    {
        ActionBase(ServiceHost* self) 
            : _self(self){}
        virtual ~ActionBase(){}

        virtual void operator()(const int* id, typename EncoderType::InputType&  params) = 0;

        virtual std::type_index typeIndex() const = 0;

        ServiceHost *_self;
    };

    template<typename TSignature>
    struct Action : ActionBase
    {

        Action(ServiceHost* self)
            : ActionBase(self) {}

        std::function<TSignature> operation;

        template<typename InputType, typename R, typename ...ArgsT>
        static R dispatch(InputType& input, std::function<R(ArgsT...)>& op)
        {

            std::tuple< typename std::decay<ArgsT>::type ... > args;

            EncoderType::parseType(input, args);

            //return grlx::callFunc(func, args);
            return std::apply(op, args);
        }        

        template<typename InputType, typename R, typename ...ArgsT>
        static void dispatchAndReply(int id, InputType& input, std::function<R(ArgsT...)>& op)
        {
            Reply<R> reply;
            reply.id = id;

            if constexpr (!std::is_void<R>::value){
                reply.result = dispatch(input, op);
            }
            else 
            {
                dispatch(input, op);
            }   

            asio::streambuf writebuf;            
            std::ostream os(&writebuf);                                             
            EncoderType::serialize(reply, os);

        }

        std::type_index typeIndex() const override
        {
            return std::type_index(typeid(TSignature));
        }

        void operator ()(const int* id, typename EncoderType::InputType& input) override
        {
            if( id == nullptr)
            {
                dispatch(input, operation);
            }
            else
            {
                dispatchAndReply(*id, input, operation);
            }
        }
    };

    using ActionPtr = std::shared_ptr< ActionBase >;

public:


    using DataHandler = std::function<int(const char*, int)>;
    

    
    ServiceHost(asio::io_context& io_context)
        : io_ctx(io_context)
    {

    }
    virtual ~ServiceHost(){}


    asio::io_context& get_io_context() 
    {
        return io_ctx;
    }


    template<typename R, typename C, typename ...ArgsT>
    void attach(std::string&& name, C* objPtr, R(C::*memFunc)(ArgsT...) const)
    {

        using ActionType = Action<R(ArgsT...)>;

        auto dispatcher = std::make_shared< ActionType >( this );

        dispatcher->operation = [&, objPtr, memFunc](ArgsT&&... args) -> R
        {
           return (objPtr->*memFunc)(args...);
        };


        this->attach(std::forward<std::string>(name), std::move(dispatcher));
    }

    template<typename R, typename C, typename ...ArgsT>
    void attach(std::string&& name, C* objPtr,  R(C::*memFunc)(ArgsT...))
    {

        using ActionType = Action<R(ArgsT...)>;

        auto dispatcher = std::make_shared< ActionType >( this );

        dispatcher->operation = [&, objPtr, memFunc](ArgsT&&... args) -> R
        {
            return (objPtr->*memFunc)(args...);
        };


        this->attach(std::forward<std::string>(name), std::move(dispatcher));
    }

    template<typename TSignature>
    void attach(std::string&& name, std::function<TSignature>&& func)
    {

        using ActionType = Action<TSignature>;

        auto dispatcher = std::make_shared< ActionType >( this );

        std::swap(dispatcher->operation, func);

        this->attach(std::forward<std::string>(name), std::move(dispatcher));
    }

    template<typename F>
    void attach(std::string&& name, F&& func)
    {
        attach(std::move(name), typename DeduceLambdaSignature<decltype(&F::operator())>::type(func));
    }


    void handleReq(const char* msg, int size)
    {
        std::string str(msg, size);
        std::stringstream is(str);

        auto handler = [this](MsgType::Type msgType, auto id, auto method, auto paramCount, auto& archive)
        {
            
            switch(msgType)
            {
                case MsgType::Invalid:
                {
                    break;
                }
                case MsgType::Request:
                {
                    auto itr = dispatchTable.find(method);
                    if(itr != dispatchTable.end()){
                        auto& action = (*itr->second);

                        action(&id,archive);
                    }
                    break;
                }
                case MsgType::Response:
                {
                    break;
                }
                case MsgType::Notification:                
                {
                    auto itr = dispatchTable.find(method);
                    if(itr != dispatchTable.end()){
                        auto& action = (*itr->second);

                        action(nullptr,archive);
                    }
                    break;
                }
                case MsgType::Error:
                {
                    break;
                }
                default:
                {
                    break;
                }
                
            }
        };
        
        EncoderType::parse(is, handler);
    }

private:

    friend EncoderType;


    template<typename Action>
    void attach(std::string&& name, std::shared_ptr<Action>&& handler)
    {
        std::string signature = handler->typeIndex().name();
        dispatchTable.emplace(std::forward<std::string>(name), handler);
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

    //TransportLayerType transportLayer;
    asio::io_context& io_ctx;
    std::unordered_map<std::string, ActionPtr> dispatchTable;
    AsyncManager<int> asyncManager;
    async::ThreadPool<> threadPool;
};


}
}
