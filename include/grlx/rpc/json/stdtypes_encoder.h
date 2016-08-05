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


#ifndef SHARE2CLOUD_RPC_STDENCODER_H
#define SHARE2CLOUD_RPC_STDENCODER_H

#include <type_traits>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iterator>

#include <grlx/utility/base64.h>
#include <grlx/utility/convert.h>

#include "encoder.h"



namespace grlx {


namespace rpc{

namespace Details {

template<>
struct JsonTypeEncoder<std::string, Details::StdContainerType>
{

    template<typename Writer>
    static void encode(std::string const& value, Writer& writer)
    {
        writer.String(value);
    }

    static bool decode(std::string& value,  rapidjson::Document::GenericValue const& jsonValue)
    {
        if(jsonValue.IsString())
        {
            value.assign(jsonValue.GetString(), jsonValue.GetStringLength());
            return true;
        }
        return false;
    }

    static bool decode(std::string& value,  rapidjson::Document::ConstValueIterator& itr)
    {
        return decode(value, *itr++);
    }

};

template<typename T, template <typename, typename = std::allocator<T>> class Container>
struct JsonTypeEncoder<Container<T>, Details::StdContainerType>
{

    template<typename Writer>
    static void encode(Container<T> const& value, Writer& writer)
    {

        writer.StartArray();

        std::for_each(value.begin(), value.end(), [&](typename Container<T>::value_type const& v)
        {
            JsonTypeEncoder<T>::encode(v, writer);
        });

        writer.EndArray();
    }

    static bool decode(Container<T>& value,  rapidjson::Document::GenericValue const& jsonValue)
    {
        if(jsonValue.GetType() == rapidjson::kArrayType)
        {
            for(auto itr = jsonValue.Begin(); itr != jsonValue.End(); ++itr)
            {
                T v;
                if(!JsonTypeEncoder<T>::decode(v, *itr))
                    return false;

                value.push_back(std::move(v));
            }
            return true;
        }
        return false;
    }

    static bool decode(Container<T>& value,  rapidjson::Document::ConstValueIterator& itr)
    {
        return decode(value, *itr++);
    }

};


template<>
struct JsonTypeEncoder<std::vector<char>, Details::StdContainerType>
{
    template<typename Writer>
    static void encode(std::vector<char> const& value, Writer& writer)
    {
        std::string hexStr(value.size()*2);
        Convert::toHex(value.begin(), value.end(), std::back_inserter(hexStr));
        writer.String(hexStr);
    }

    static bool decode(std::vector<char>& value,  rapidjson::Document::GenericValue const& jsonValue)
    {
        auto size = jsonValue.GetStringLength();
        auto begin = jsonValue.GetString();
        auto end = begin + size;
        Convert::fromHex(begin, end, std::back_inserter(value));
        return true;
    }

    static bool decode(std::vector<char>& value,  rapidjson::Document::ConstValueIterator& itr)
    {
        return decode(value, *itr++);
    }


};

template<>
struct JsonTypeEncoder<std::vector<uint8_t>, Details::StdContainerType>
{
    template<typename Writer>
    static void encode(std::vector<uint8_t> const& value, Writer& writer)
    {

        std::string result;
        base64::encode(value.begin(), value.end(), std::back_inserter(result));
        writer.String(result);
    }

    static bool decode(std::vector<uint8_t>& value,  rapidjson::Document::GenericValue const& jsonValue)
    {
        auto begin = jsonValue.GetString();
        auto size = jsonValue.GetStringLength();
        auto end = begin + size;
        base64::decode(begin, end, std::back_inserter(value));
        return true;
    }

    static bool decode(std::vector<uint8_t>& value,  rapidjson::Document::ConstValueIterator& itr)
    {
        return decode(value, *itr++);
    }


};


template<typename K, typename U, template<class...> class ContainerType>
struct JsonTypeEncoder<ContainerType<K,U>, Details::StdContainerType >
{

    template<typename Writer>
    static void encode(ContainerType<K, U> const& value, Writer& stream)
    {
//        Details::encodeTypeID< ContainerType<K,U> >(stream);
//        Details::encodeTypeID<K>(stream);
//        Details::encodeTypeID<U>(stream);
//        Details::encodeValue(value.size(), stream);

//        std::for_each(value.begin(), value.end(), [&](typename ContainerType<K,U>::value_type const& v)
//        {
//            Details::encodeValue(v.first, stream);
//            Details::encodeValue(v.second, stream);
//        });
    }
//    static bool decode(ContainerType<K, U>& value, StreamT& stream)
//    {
////        int containerTypeID = Details::decodeTypeID(stream);

////        if(containerTypeID != TypeID<ContainerType<K,U>>::value)
////            return false;

////        int keyTypeID = Details::decodeTypeID(stream);

////        if(keyTypeID != TypeID<K>::value)
////            return false;

////        int valueTypeID = Details::decodeTypeID(stream);

////        if(valueTypeID != TypeID<U>::value)
////            return false;


////        std::size_t size = Details::decodeValue<std::size_t>( stream);

////        bool sucess = true;

////        for(size_t i = 0; i < size; ++i)
////        {
////            K key; U val;
////            sucess &= Details::decodeValue(key, stream);

////            if(!sucess)
////                break;

////            sucess &= Details::decodeValue(val, stream);

////            if(!sucess)
////                break;

////            value.insert(std::make_pair(std::move(key), std::move(val)));

////        }
////        return sucess;
//    }
};

}

}
}
#endif // SHARE2CLOUD_RPC_STDENCODER_H
