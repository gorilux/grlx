#pragma once

#include <sstream>
#include <iomanip>
#include <iostream>


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



#include "grlx/rpc/types.h"
#include "grlx/rpc/message.h"

namespace grlx
{
namespace rpc
{

template<typename TOuputArchive, typename TInputArchive>
class GenericEncoder
{

public:

    using ParamsType = TInputArchive;
    using ResultType = TInputArchive;

    template<typename TMsg, typename TSenderF>
    static void encode(TMsg const& msg, TSenderF send)
    {

        std::ostringstream os;
        {
            TOuputArchive archive(os);
            encodeMsg(msg, archive);
        }

        auto buffer = os.str();

        send(buffer.data(), buffer.size());

    }
    template<typename Archive, typename ...TArgs>
    static void encodeMsg(Request<TArgs...> const& msg, Archive& archive)
    {
        archive( cereal::make_nvp("msgtype", MsgType::Request ) );
        archive( cereal::make_nvp("id", msg.id ));
        archive( cereal::make_nvp("method", msg.method ));
        archive( cereal::make_nvp("params", msg.args ));

    }

    template<typename Archive, typename ...TArgs>
    static void encodeMsg(Notification<TArgs...> const& msg, Archive& archive)
    {
        archive( cereal::make_nvp("msgtype", MsgType::Notification ) );
        archive( cereal::make_nvp("method", msg.method ));
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



    template<typename Ch, typename HandlerType>
    static bool decodeReq(Ch* buffer, int size, HandlerType& handler, TokenType userToken)
    {
        std::string str(buffer, size);
        std::istringstream is( str );

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
                    archive( cereal::make_nvp("id", id ) );
                    archive( cereal::make_nvp("method", method ) );

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
                    archive( cereal::make_nvp("method", method ) );
                    // archive( cereal::make_nvp("params", params ) );
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
        }
        return false;
    }

    template<typename Ch, typename HandlerType>
    static bool decodeResp(Ch* buffer, int size, HandlerType& handler)
    {
        std::string str(buffer, size);
        std::istringstream is( str );

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

    template<typename TParams, typename T>
    static bool decodeType(TParams& archive, T& t)
    {
        archive(t);
        return true;
    }

};




}

}
