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

#ifndef GRLX_SIGNAL_PROPERTY_H
#define GRLX_SIGNAL_PROPERTY_H


#include <functional>
#include <type_traits>


#include "signal.h"


namespace grlx
{


template<typename ValueT>
class Property
{

public:

    using ValueType = ValueT;

    Signal<void(Property const&)> Changed;

    Property()
    {
    }


    Property(ValueT const& value)
        : value( value )
        , isSet( true )
    {

    }

    Property(ValueT&& value)
        : value( std::move(value) )
        , isSet( true )
    {

    }

    ~Property()
    {

    }

    explicit Property(Property const& copy)
    {
        *this = copy;
    }


     Property<ValueT>& operator= (const Property<ValueT>& other)
     {

         this->value = other.value;
         this->isSet = other.isSet;
         Changed.Emit(*this);
         return *this;
     }

    template<typename T>
    Property<ValueT>& operator= (const T& value)
    {
        this->value = value;
        isSet = true;       
        Changed.Emit(*this);
        return *this;
    }

    bool operator == ( const Property<ValueT>& other) const
    {
        return other.value == this->value && other.isSet == this->isSet;
    }
    template<typename T>
    bool operator == (const T& other) const
    {
        return other == this->value;
    }
    template<typename T>
    bool operator != (const T& other) const
    {
        return other == this->value;
    }


    operator const ValueT&() const noexcept
    {
        return value;
    }



    bool IsSet()
    {
        return isSet;
    }
    void Clear()
    {
        isSet = false;
        Changed.Emit(*this);
    }

    const ValueT& Value() const
    {
        return value;
    }

    ValueT& Value()
    {
        return value;
    }

    template<class Archive>
    void save(Archive& ar) const
    {
        ar(value);
        ar(isSet);
    }

    template<class Archive>
    void load(Archive& ar)
    {
        ar(value);
        ar(isSet);
        Changed.Emit(*this);
    }

private:
    ValueT value;
    bool isSet;
};

}

#endif // PROPERTY_H
