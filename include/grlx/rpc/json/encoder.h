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

#ifndef GRLX_RPC_JSON_ENCODER_H
#define GRLX_RPC_JSON_ENCODER_H

#include <type_traits>
#include <tuple>
#include <memory>
#include <iterator>
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <rapidjson/reader.h>
#include <rapidjson/memorybuffer.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/document.h>

#include <grlx/rpc/types.h>
#include <grlx/rpc/utility.h>
#include <grlx/rpc/message.h>


namespace grlx {


namespace rpc{

namespace Details
{


template<typename T, typename TagT = typename TypeID<T>::Tag >
struct JsonTypeEncoder;


template<typename T>
class Packer
{

public:
    Packer(T& t)
        : t_(t){}

    template<typename...TArgs>
    void pack(TArgs const&... args)
    {
        packHelper(t_,args...);
    }

    template<typename...TArgs>
    bool unpack(TArgs&... args)
    {
        return unpackHelper(t_, args...);
    }
private:

    template<typename Writer, typename U, typename ...TArgs>
    static void packHelper(Writer& writer, U const& u, TArgs const& ...args)
    {
        typedef typename std::remove_reference<U>::type V;
        JsonTypeEncoder<typename std::remove_cv<V>::type >::encode(u, writer);

        packHelper(writer, args...);
    }
    template<typename Writer>
    static void packHelper(Writer&) { }


    template<typename Reader, typename U, typename ...TArgs>
    static bool unpackHelper(Reader& reader, U& u, TArgs& ...args)
    {
        typedef typename std::remove_reference<U>::type V;
        bool result = JsonTypeEncoder<typename std::remove_cv<V>::type >::decode(u, reader);

        result &= unpackHelper(reader, args...);
        return result;
    }

    template<typename Reader>
    static bool unpackHelper(Reader&) { return true; }


    T& t_;
};



template<typename ...ArgsT>
struct JsonTypeEncoder< Request<ArgsT...>, Details::RequestType >
{
    template<typename Writer>
    static void encode(Request<ArgsT...> const& msg, Writer& writer)
    {

        using ArgsTuple = typename Request<ArgsT...>::ArgsType;

        writer.StartObject();
        writer.Key("jsonrpc");
        writer.String("2.0");
        writer.Key("method");
        writer.String(msg.method);

        writer.Key("params");
        writer.StartArray();
        JsonTypeEncoder<ArgsTuple>::encode(msg.args, writer);
        writer.EndArray();

        writer.Key("id");
        writer.Int(msg.id);
        writer.EndObject();
    }

};


template<typename T>
struct JsonTypeEncoder< Reply<T>, Details::ReplyType >
{
    template<typename Writer>
    static void encode(Reply<T> const& msg, Writer& writer)
    {


        writer.StartObject();
        writer.Key("jsonrpc");
        writer.String("2.0");

        writer.Key("result");
        JsonTypeEncoder<T>::encode(msg.result, writer);

        writer.Key("id");
        writer.Int(msg.id);
        writer.EndObject();
    }

};

template<>
struct JsonTypeEncoder< Reply<void>, Details::ReplyType >
{
    template<typename Writer>
    static void encode(Reply<void> const& msg, Writer& writer)
    {
        writer.StartObject();
        writer.Key("jsonrpc");
        writer.String("2.0");

        writer.Key("result");
        writer.Null();

        writer.Key("id");
        writer.Int(msg.id);
        writer.EndObject();
    }

};

template<typename ...ArgsT>
struct JsonTypeEncoder< Notification<ArgsT...>, Details::NotificationType >
{
    template<typename Writer>
    static void encode(Notification<ArgsT...> const& msg, Writer& writer)
    {

        using ArgsTuple = typename Request<ArgsT...>::ArgsType;

        writer.StartObject();
        writer.Key("jsonrpc");
        writer.String("2.0");
        writer.Key("method");
        writer.String(msg.method);

        writer.Key("params");
        writer.StartArray();
        JsonTypeEncoder<ArgsTuple>::encode(msg.args, writer);
        writer.EndArray();


        writer.EndObject();
    }

};



template<typename ...ArgsT>
struct JsonTypeEncoder<std::tuple<ArgsT...>, Details::TupleType>
{
    using TupleT = std::tuple<ArgsT...>;

    template<typename T>
    using EncoderType = JsonTypeEncoder<T>;

    template<typename Writer>
    static void encode(TupleT const& args, Writer& writer)
    {

        TupleEncoder<EncoderType, TupleT >::encode(args, writer);

    }

//    template<typename Reader>
//    static bool decode(TupleT& args, Reader& reader)
//    {
//        return TupleEncoder<EncoderType, TupleT >::decode(args, reader);
//    }

    static bool decode(TupleT& args, rapidjson::Document::ConstValueIterator& itr)
    {
        bool ret = TupleEncoder<EncoderType, TupleT >::decode(args, itr);
        return ret;
    }
};


template<typename T>
struct JsonTypeEncoder<T, Details::FundamentalType>
{


    template<typename Writer>
    static void encode(uint64_t value, Writer& writer)
    {
        writer.Uint64(value);
    }
    template<typename Writer>
    static void encode(int64_t value, Writer& writer)
    {
        writer.Int64(value);
    }
    template<typename U, typename Writer>
    static typename std::enable_if<
        std::is_unsigned<U>::value && std::is_integral<U>::value
    >::type
    encode(U value, Writer& writer)
    {
        writer.Uint(value);
    }

    template<typename U, typename Writer>
    static typename std::enable_if<
        std::is_signed<U>::value &&  std::is_integral<U>::value
    >::type
    encode(U value, Writer& writer)
    {
        writer.Int(value);
    }

    template<typename U, typename Writer>
    static typename std::enable_if<
        std::is_floating_point<U>::value
    >::type
    encode(U value, Writer& writer)
    {
        writer.Double(value);
    }

    static bool decode(uint64_t& value, rapidjson::Document::GenericValue const& jsonValue)
    {
        value = jsonValue.GetUint64();
        return true;
    }

    static bool decode(int64_t& value, rapidjson::Document::GenericValue const& jsonValue)
    {
        value = jsonValue.GetInt64();
        return true;
    }

    template<typename U>
    static typename std::enable_if<
        std::is_unsigned<U>::value && std::is_integral<U>::value,
        bool
    >::type
    decode(U& value, rapidjson::Document::GenericValue const& jsonValue)
    {
        //writer.UInt(value);
        value = jsonValue.GetUint();
        return true;
    }

    template<typename U>
    static typename std::enable_if<
        std::is_signed<U>::value &&  std::is_integral<U>::value,
        bool
    >::type
    decode(U& value, rapidjson::Document::GenericValue const& jsonValue)
    {
        //writer.Int(value);
        value = jsonValue.GetInt();
        return true;
    }

    template<typename U>
    static typename std::enable_if<
        std::is_floating_point<U>::value,
        bool
    >::type
    decode(U& value, rapidjson::Document::GenericValue const& jsonValue)
    {
        value = jsonValue.GetDouble();

        return true;
    }

    static bool decode(T& value,  rapidjson::Document::ConstValueIterator& itr)
    {
        return decode(value, *itr++);
    }
};


template<int N>
struct JsonTypeEncoder<char[N], Details::FundamentalType>
{
    template<typename Writer>
    static void encode(const char value[N], Writer& writer)
    {
        writer.String(value);
    }
};

template<>
struct JsonTypeEncoder<const char*, Details::FundamentalType>
{
    template<typename Writer>
    static void encode(const char* value, Writer& writer)
    {
        writer.String(value);
    }


    static bool decode(const char*& value,  rapidjson::Document::GenericValue const& jsonValue)
    {
        value = jsonValue.GetString();
        return true;
    }

    static bool decode(const char*& value,  rapidjson::Document::ConstValueIterator& itr)
    {
        return decode(value, *itr++);
    }
};



template<typename T>
struct JsonTypeEncoder<T, Details::ComplexType>
{

    template<typename Writer>
    static void encode(T const& t, Writer& writer)
    {

//        writer.StartObject();

//        writer.Key("Obj");

        writer.StartArray();

        Packer<Writer> packer(writer);

        t.pack(packer);

        writer.EndArray();

//        writer.EndObject();

    }


    static bool decode(T& t,  rapidjson::Document::GenericValue const& jsonValue)
    {
        if(jsonValue.GetType() == rapidjson::kArrayType)
        {
            auto itr = jsonValue.Begin();

            Packer<rapidjson::Document::ConstValueIterator> packer(itr);

            return t.unpack(packer);
        }
        return false;

    }
    static bool decode(T& t,  rapidjson::Document::ConstValueIterator& itr)
    {
        return decode(t, *itr++);
    }
};



}

class JsonEncoder
{
public:


    typedef rapidjson::GenericValue< rapidjson::UTF8<> > ParamsType;
    typedef rapidjson::GenericValue< rapidjson::UTF8<> > ResultType;


    template<typename TMsg, typename TSenderF>
    static void encode(TMsg const& msg, TSenderF send)
    {

        rapidjson::MemoryBuffer buffer;

        rapidjson::Writer<rapidjson::MemoryBuffer> writer(buffer);

        Details::JsonTypeEncoder<TMsg>::encode(msg, writer);

        send(buffer.GetBuffer(), buffer.GetSize());
    }


    template<typename InputStream, typename ServiceProviderType>
    static bool decodeReq(InputStream& is, ServiceProviderType& serviceProvider)
    {

        rapidjson::Document document;

        document.ParseStream<rapidjson::kParseIterativeFlag, rapidjson::Document::EncodingType, InputStream>(is);

        if(document["jsonrpc"] != "2.0")
        {
            // handler.error
            return false;
        }

        auto itr = document.FindMember("method");
        if(itr != document.MemberEnd())
        {
            auto& methodRef = itr->value;

            std::string method(methodRef.GetString(), methodRef.GetStringLength());

            itr = document.FindMember("id");

            if(itr != document.MemberEnd())
            {
                //request
                auto& idRef = itr->value;

                itr = document.FindMember("params");
                if( itr != document.MemberEnd())
                {
                    auto& paramsRef = itr->value;

                    serviceProvider.exec(method, idRef.GetInt(), paramsRef);
                }
                else
                {
                    serviceProvider.error(method, idRef.GetInt());
                }

            }
            else
            {
                //notification
                itr = document.FindMember("params");
                if( itr != document.MemberEnd())
                {
                    auto& paramsRef = itr->value;
                    serviceProvider.exec(method, paramsRef);
                }
                else
                {
                    serviceProvider.error(method);
                }

            }

        }

        // itr = document.FindMember("result");
        // if(itr != document.MemberEnd())
        // {
        //     auto& resultsRef = itr->value;

        //     itr = document.FindMember("id");
        //     if(itr != document.MemberEnd())
        //     {
        //         auto& idRef = itr->value;
        //         serviceProvider.reply(idRef.GetInt(), resultsRef);
        //         return true;
        //     }
        // }

        return true;
    }

    template<typename InputStream, typename HandlerType>
    static bool decodeResp(InputStream& is, HandlerType& handler)
    {

        rapidjson::Document document;

        document.ParseStream<rapidjson::kParseIterativeFlag, rapidjson::Document::EncodingType, InputStream>(is);

        if(document["jsonrpc"] != "2.0")
        {
            // handler.error
            return false;
        }


        auto itr = document.FindMember("result");
        if(itr != document.MemberEnd())
        {
            auto& resultsRef = itr->value;

            itr = document.FindMember("id");
            if(itr != document.MemberEnd())
            {
                auto& idRef = itr->value;
                handler.reply(idRef.GetInt(), resultsRef);
            }

        }
        return true;
    }

    template<typename Ch, typename HandlerType>
    static bool decodeReq(Ch* buffer, int size, HandlerType& handler)
    {
        rapidjson::MemoryStream inputStream(buffer, size);

        return decodeReq(inputStream, handler);
    }

    template<typename Ch, typename HandlerType>
    static bool decodeResp(Ch* buffer, int size, HandlerType& handler)
    {
        rapidjson::MemoryStream inputStream(buffer, size);

        return decodeResp(inputStream, handler);
    }

    template<typename TParams, typename ...TArgs>
    static bool decodeType(TParams const& params, std::tuple<TArgs...>& t)
    {
        switch(params.GetType()){
            case rapidjson::kArrayType:
            {
                auto itr = params.Begin();
                return Details::JsonTypeEncoder<std::tuple<TArgs...>>::decode(t, itr);
            }
            default:
                return false;
        }
    }

    template<typename TJSonValue, typename T>
    static bool decodeType(TJSonValue const& jsonValue, T& t)
    {


        switch(jsonValue.GetType()){
            case rapidjson::kArrayType:
            case rapidjson::kObjectType:
            case rapidjson::kStringType:
            case rapidjson::kNumberType:
                 return Details::JsonTypeEncoder<T>::decode(t, jsonValue);
            case rapidjson::kNullType:
            case rapidjson::kFalseType:
            case rapidjson::kTrueType:
            default:
                return false;
        }
    }

};


} // namespace RPC
}

#include "stdtypes_encoder.h"

#endif // SHARE2CLOUD_RPC_ENCODER_H
