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


#include <sstream>
#include <iomanip>
#include <iostream>
#include <type_traits>


#include <cereal/types/array.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/forward_list.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>

#include <cereal/details/util.hpp>

#include <cereal/details/polymorphic_impl.hpp>
#include <cereal/types/polymorphic.hpp>

#include <cereal/types/queue.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/stack.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/variant.hpp>



#include "grlx/rpc/types.h"
#include "grlx/rpc/message.h"

namespace grlx
{
namespace rpc
{

namespace details
{
template<class, class = std::void_t<> >
struct HasMsgType : std::false_type{};

template<class T>
struct HasMsgType <T, std::void_t<typename T::MsgType>> : std::true_type{};

}

template<typename TOuputArchive, typename TInputArchive>
class GenericEncoder
{

public:

    using ParamsType = TInputArchive;
    using ResultType = TInputArchive;

//    template<typename TMsg, typename TOStream>
//    static void encode(TMsg const& msg, TOStream& os)
//    {

//        TOuputArchive archive(os);
//        encodeMsg(msg, archive);
//    }


    template<typename T, typename TOStream>
    static void encode(T const& t, TOStream& os)
    {
        TOuputArchive archive(os);
        if constexpr (details::HasMsgType<T>::value)
        {
            archive( cereal::make_nvp("msgtype", t.MsgType ) );
        }

        archive(t);
    }



    template<typename Archive, typename ...TArgs>
    static void encodeMsg(Request<TArgs...> const& msg, Archive& archive)
    {
        archive( cereal::make_nvp("msgtype", MsgType::Request ) );
        archive( cereal::make_nvp("id", msg.id ));
        archive( cereal::make_nvp("method", msg.method ));
        archive( cereal::make_nvp("paramcount", msg.numArgs ));
        archive( cereal::make_nvp("params", msg.args ));


    }

    template<typename Archive, typename ...TArgs>
    static void encodeMsg(Notification<TArgs...> const& msg, Archive& archive)
    {
        archive( cereal::make_nvp("msgtype", MsgType::Notification ) );
        archive( cereal::make_nvp("method", msg.method ));
        archive( cereal::make_nvp("paramcount", msg.numArgs ));
        archive( cereal::make_nvp("params", msg.args ));
    }

    template<typename Archive, typename TResult>
    static void encodeMsg(Reply<TResult> const& msg, Archive& archive)
    {
        archive( cereal::make_nvp("msgtype", MsgType::Response ) );
        archive( cereal::make_nvp("id", msg.id ));
        archive( cereal::make_nvp("result", msg.result ));
    }

    template<typename Archive>
    static void encodeMsg(Reply<void> const& msg, Archive& archive)
    {
        archive( cereal::make_nvp("msgtype", MsgType::Response ) );
        archive( cereal::make_nvp("id", msg.id ));
    }



    template<typename TIStream, typename HandlerType>
    static bool decodeReq(TIStream& is, HandlerType& handler, TokenType userToken)
    {

        TInputArchive archive(is);
        MsgType::Type msgType;

        archive( cereal::make_nvp("msgtype", msgType ) );

        switch(msgType)
        {
            case MsgType::Invalid:
            {
                break;
            }
            case MsgType::Request:
            {


                int id;
                std::string method;
                int paramCount;
                std::string signature;
                archive( cereal::make_nvp("id", id ) );
                archive( cereal::make_nvp("method", method ) );
                archive( cereal::make_nvp("paramcount",  paramCount));
                handler.exec(method, id, archive, userToken);


                break;
            }
            case MsgType::Response:
            {
                break;
            }
            case MsgType::Notification:
            {
                std::string method;
                int paramCount;
                archive( cereal::make_nvp("method", method ) );
                archive( cereal::make_nvp("paramcount",  paramCount));
                handler.exec(method, archive, userToken);
                break;
            }
            case MsgType::Error:
            {
                break;
            }
            default:
                break;
        }

        return false;
    }

    template<typename TIStream, typename HandlerType>
    static bool decodeResp(TIStream& is, HandlerType& handler)
    {

        TInputArchive archive(is);

        MsgType::Type msgType;

        archive( cereal::make_nvp("msgtype", msgType ) );

        switch(msgType)
        {
            case MsgType::Invalid:
            {
                break;
            }
            case MsgType::Request:
            {
                break;
            }
            case MsgType::Response:
            {
                int id;
                archive( cereal::make_nvp("id", id ) );
                // archive( cereal::make_nvp("result", result ) );
                // archive( cereal::make_nvp("params", params ) );

                handler.reply(id, archive);
                break;
            }
            case MsgType::Notification:
            {
                break;
            }
            case MsgType::Error:
            {
                break;
            }
            default:
            break;
        }
        return true;
    }

    template<typename TArchive, typename T>
    static bool decodeType(TArchive& archive, T& t)
    {
        archive(t);
        return true;
    }

};




}

}
