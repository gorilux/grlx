#pragma once

#include <QString>

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
