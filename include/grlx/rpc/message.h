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


#include <string>
#include <tuple>
#include <cereal/cereal.hpp>
#include "types.h"

namespace grlx {

namespace rpc
{

template<typename TReturn, typename ...TArgs>
struct Request
{
    static constexpr MsgType::Type Type = MsgType::Request;

    using ArgsType   = std::tuple<TArgs...>;
    using ReturnType = TReturn;

    std::type_index typeIndex = typeid(ReturnType(TArgs...));

    Request(std::string&& method, int id, TArgs&&... args)
        : method(method), id(id), args(std::forward_as_tuple(args...)){}

    Request(){}

    constexpr static int  numArgs = sizeof ... (TArgs);
    std::string method;
    int id;
    ArgsType    args;

    template<class Archive>
    void serialize(Archive& archive)
    {       
        archive( cereal::make_nvp("id"        , id ));
        archive( cereal::make_nvp("method"    , method ));
        archive( cereal::make_nvp("paramcount", numArgs ));
        archive( cereal::make_nvp("params"    , args ));
    }

//    template <class Archive>
//    static void load_and_construct( Archive & archive, cereal::construct<Request<TArgs...> > & construct )
//    {

//    }

};


template<typename TResult>
struct Reply
{
    static constexpr MsgType::Type Type = MsgType::Response;

    int id;
    TResult result;

    template<class Archive>
    void serialize(Archive& archive)
    {        
        archive( cereal::make_nvp("id", id ));
        archive( cereal::make_nvp("result", result ));
    }

    template <class Archive>
    static void load_and_construct( Archive & archive, cereal::construct<Reply<TResult> > & construct )
    {

    }
};

template<>
struct Reply<void>
{
    static constexpr MsgType::Type Type = MsgType::Response;

    int id;

    template<class Archive>
    void serialize(Archive& archive)
    {                
        archive( cereal::make_nvp("id", id ));
    }

    template <class Archive>
    static void load_and_construct( Archive & archive, cereal::construct<Reply<void> > & construct )
    {

    }
};


template<typename ...TArgs>
struct Notification
{

    static constexpr MsgType::Type Type = MsgType::Notification;

    using ArgsType = std::tuple<TArgs...>;

    std::type_index typeIndex = typeid(void(TArgs...));

    Notification(std::string&& method, TArgs&&... args)
        : method(method), args(std::forward_as_tuple(args...)){}

    constexpr static int numArgs = sizeof ... (TArgs);
    std::string          method;
    ArgsType             args;

    template<class Archive>
    void serialize(Archive& archive)
    {        
        archive( cereal::make_nvp("method", method ));
        archive( cereal::make_nvp("paramcount", numArgs ));
        archive( cereal::make_nvp("params", args ));
    }

    template <class Archive>
    static void load_and_construct( Archive & archive, cereal::construct<Notification<TArgs...> > & construct )
    {

    }



};


struct Error
{
    int id;
    int error;
    std::string message;

    template<class Archive>
    void serialize(Archive& archive)
    {

    }

    template <class Archive>
    static void load_and_construct( Archive & archive, cereal::construct<Error> & construct )
    {

    }
};


}
}

