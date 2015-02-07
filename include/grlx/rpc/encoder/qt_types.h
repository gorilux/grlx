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



#include <cereal/cereal.hpp>

#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QVariant>
#include <QHash>
#include <QVector>

template<typename Archive>
inline void save( Archive & archive, QString const& str )
{
    archive( str.toStdString() );
}


template<typename Archive>
inline void load( Archive & archive, QString& str )
{
    std::string v;

    archive(v);
    str = QString::fromStdString(v);
}



template<class Archive>
void load(Archive & archive, QByteArray& byteArray)
{
    cereal::size_type size;
    archive( cereal::make_size_tag( size ) );
    byteArray.resize(  static_cast<cereal::size_type>(size)  );
    archive( cereal::binary_data( byteArray.data(), byteArray.size() * sizeof(char) ) );
}

template<class Archive>
void save(Archive & archive, QByteArray const& byteArray)
{
    archive( cereal::make_size_tag( static_cast<cereal::size_type>(byteArray.size()) ) ); // number of elements
    archive( cereal::binary_data( byteArray.data(), byteArray.size() * sizeof(char) ) );
}


template<class Archive>
void load(Archive & archive, QVariant& m)
{
    QByteArray data;
    archive(data);
    {
        QDataStream s(&data, QIODevice::ReadOnly);
        s.setVersion(QDataStream::Qt_5_11);
        s >> m;
    }
}

template<class Archive>
void save(Archive & archive, QVariant const& m)
{
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::ReadWrite);
        s.setVersion(QDataStream::Qt_5_11);
        s << m;
    }
    archive(data);
}

template<class Archive, typename K, typename V>
void load(Archive & archive, QHash<K,V>& map)
{
    cereal::size_type  size;
    archive( cereal::make_size_tag( size ) );
    map.clear();
    for( size_t i = 0; i < size; ++i )
    {
        K key;
        V value;
        archive( cereal::make_map_item(key, value) );
        map.insert(key,value);
    }

}

template<class Archive, typename K, typename V>
void save(Archive & archive, QHash<K,V> const& map)
{
    archive( cereal::make_size_tag(static_cast<cereal::size_type>(map.size()) ) );

    for( const auto & k : map.keys() )
    {
        archive(cereal::make_map_item(k, map.value(k)) );
    }

}


template<class Archive, typename T>
void load(Archive & archive, QVector<T>& vector)
{
    cereal::size_type size;
    archive( cereal::make_size_tag( size ) );

    vector.resize(size);
    for( auto&& v: vector )
    {
        archive( v );
    }
}

template<class Archive, typename T>
void save(Archive & archive,  QVector<T> const& vector)
{
    archive( cereal::make_size_tag( static_cast<cereal::size_type>(vector.size()) ) ); // number of elements

    for(auto && v : vector)
    {
        archive( v );
    }
}

