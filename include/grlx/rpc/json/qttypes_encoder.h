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

#ifndef GRLX_RPC_QTTYPEENCODER_H
#define GRLX_RPC_QTTYPEENCODER_H

#include <QString>
#include <QVector>
#include <QList>
#include <QSet>
#include <QMap>
#include <QMultiMap>
#include <QHash>
#include <QMultiHash>
#include <QQueue>
#include <QByteArray>

#include <algorithm>
#include <iterator>


#include <grlx/rpc/json/encoder.h>

namespace grlx {


namespace rpc{

namespace Details
{
    struct QtContainerType{};
}

template<>
struct TypeID<QString> : IntConst<kQtLibTypes + 0>
{
    typedef Details::QtContainerType Tag;
};

template<>
struct TypeID<QByteArray> : IntConst<kQtLibTypes + 1>
{
    typedef Details::QtContainerType Tag;
};


template<typename T>
struct TypeID<QVector<T>> : IntConst<kQtLibTypes + 2>
{
    typedef Details::QtContainerType Tag;
};

template<typename T>
struct TypeID<QList<T>> : IntConst<kQtLibTypes + 3>
{
    typedef Details::QtContainerType Tag;
};

template<typename T>
struct TypeID<QQueue<T>> : IntConst<kQtLibTypes + 4>
{
    typedef Details::QtContainerType Tag;
};

template<typename T>
struct TypeID<QSet<T>> : IntConst<kQtLibTypes + 5>
{
    typedef Details::QtContainerType Tag;
};

template<typename K, typename U>
struct TypeID<QMap<K,U>> : IntConst<kQtLibTypes + 6>
{
    typedef Details::QtContainerType Tag;
};
template<typename K, typename U>
struct TypeID<QMultiMap<K,U>> : IntConst<kQtLibTypes + 7>
{
    typedef Details::QtContainerType Tag;
};
template<typename K, typename U>
struct TypeID<QHash<K,U>> : IntConst<kQtLibTypes + 8>
{
    typedef Details::QtContainerType Tag;
};
template<typename K, typename U>
struct TypeID<QMultiHash<K,U>> : IntConst<kQtLibTypes + 9>
{
    typedef Details::QtContainerType Tag;
};

namespace Details {

template<>
struct JsonTypeEncoder<QString, Details::QtContainerType>
{


    template<typename Writer>
    static void encode(QString const& value, Writer& writer)
    {

        writer.String(value.toUtf8().constData());

    }

    static bool decode(QString& value,  rapidjson::Document::GenericValue const& jsonValue)
    {
        value = jsonValue.GetString();
        return true;
    }

    static bool decode(QString& value,  rapidjson::Document::ConstValueIterator& itr)
    {
        return decode(value, *itr++);
    }
};

template<>
struct JsonTypeEncoder<QByteArray, Details::QtContainerType>
{

    template<typename Writer>
    static void encode(QByteArray const& value, Writer& writer)
    {
        writer.String(value.toHex().constData());
    }

    static bool decode(QByteArray& value,  rapidjson::Document::GenericValue const& jsonValue)
    {
        value = QByteArray::fromHex(jsonValue.GetString());
        return true;
    }

    static bool decode(QByteArray& value,  rapidjson::Document::ConstValueIterator& itr)
    {
        return decode(value, *itr++);
    }
};

template<typename T, template<class> class ContainerType >
struct JsonTypeEncoder<ContainerType<T>, Details::QtContainerType>
{

    template<typename Writer>
    static void encode(ContainerType<T> const& value, Writer& writer)
    {
        writer.StartArray();

        std::for_each(value.begin(), value.end(), [&](typename ContainerType<T>::value_type const& v)
        {
            JsonTypeEncoder<T>::encode(v, writer);
        });

        writer.EndArray();

    }
//    static bool decode(ContainerType<T>& value, StreamT& stream)
//    {
//        int containerTypeID = Details::decodeTypeID(stream);

//        if(containerTypeID != TypeID<ContainerType<T>>::value)
//            return false;

//        int elementTypeID = Details::decodeTypeID(stream);

//        if(elementTypeID != TypeID<T>::value)
//            return false;

//        std::size_t size = Details::decodeValue<std::size_t>( stream);


//        bool isSucess = true;

//        for(size_t i = 0; i < size; ++i)
//        {
//            T v;
//            isSucess &= Details::decodeValue(v, stream);

//            if(!isSucess)
//                break;

//            value.push_back(std::move(v));

//        }
//        return isSucess;

//    }
};


template<typename K, typename U, template<class...> class ContainerType>
struct JsonTypeEncoder<ContainerType<K,U>, Details::QtContainerType >
{

    template<typename Writer>
    static void encode(ContainerType<K, U> const& value, Writer& writer)
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
//        int containerTypeID = Details::decodeTypeID(stream);

//        if(containerTypeID != TypeID<ContainerType<K,U>>::value)
//            return false;

//        int keyTypeID = Details::decodeTypeID(stream);

//        if(keyTypeID != TypeID<K>::value)
//            return false;

//        int valueTypeID = Details::decodeTypeID(stream);

//        if(valueTypeID != TypeID<U>::value)
//            return false;


//        std::size_t size = Details::decodeValue<std::size_t>( stream);

//        bool sucess = true;

//        for(size_t i = 0; i < size; ++i)
//        {
//            K key; U val;
//            sucess &= Details::decodeValue(key, stream);

//            if(!sucess)
//                break;

//            sucess &= Details::decodeValue(val, stream);

//            if(!sucess)
//                break;

//            value.insert(std::make_pair(std::move(key), std::move(val)));

//        }
//        return sucess;
//    }
};

}

}
}

#endif // QTTYPEENCODER_H
