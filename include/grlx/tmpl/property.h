/* 	 
*    Copyright (c) 2011 David Salvador Pinheiro
*
*    http://www.gorilux.org 
*       
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.  
*/

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
