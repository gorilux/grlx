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

#ifndef GRLX_TYPEID
#define GRLX_TYPEID

namespace grlx
{


typedef unsigned long TypeId;

template<class T>
struct TypeInfo
{
    static TypeId id() { return reinterpret_cast<TypeId>( &inst() ); }
private:
    struct Impl{};
    static Impl const & inst() { static Impl sImpl; return sImpl; }
};

template<class T>
TypeId typeId()
{
    return TypeInfo<T>::id();
}

template<class T>
TypeId typeId(T)
{
    return TypeInfo<T>::id();
}

}
#endif
