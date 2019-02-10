#pragma once

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
    size_t size;
    archive( cereal::make_size_tag( size ) );
    byteArray.resize(  size  );
    for(auto && v : byteArray)
      archive( v );
}

template<class Archive>
void save(Archive & archive, QByteArray const& byteArray)
{
    archive( cereal::make_size_tag( static_cast<size_t>(byteArray.size()) ) ); // number of elements
    for(auto && v : byteArray)
      archive( v );
}


template<class Archive>
void load(Archive & archive, QVariant& m)
{
    QByteArray data;
    archive(data);
    {
        QDataStream s(&data, QIODevice::ReadOnly);
        s >> m;
    }
}

template<class Archive>
void save(Archive & archive, QVariant const& m)
{
    QByteArray data;
    {
        QDataStream s(&data, QIODevice::ReadWrite);
        s << m;
    }
    archive(data);
}

template<class Archive, typename K, typename V>
void load(Archive & archive, QHash<K,V>& map)
{
    size_t size;
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
    archive( cereal::make_size_tag( static_cast<size_t>(map.size()) ) );

    for( const auto & k : map.keys() )
    {
        archive(cereal::make_map_item(k, map.value(k)) );
    }

}


template<class Archive, typename T>
void load(Archive & archive, QVector<T>& vector)
{
    size_t size;
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
    archive( cereal::make_size_tag( static_cast<size_t>(vector.size()) ) ); // number of elements

    for(auto && v : vector)
    {
        archive( v );
    }
}

