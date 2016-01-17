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

#ifndef GRLX_TMPL_ITER_H
#define GRLX_TMPL_ITER_H

#include "tags.h"

namespace grlx
{

template<typename TAG>
struct BeginImpl
{
    template<typename Sequence>
    struct Apply;
};

template<typename Sequence>
struct Begin : BeginImpl< typename Tag<Sequence>::Type >::template
        Apply<Sequence>
{
};

template<typename TAG>
struct EndImpl
{
    template<typename Sequence>
    struct Apply;
};

template<typename Sequence>
struct End : EndImpl< typename Tag<Sequence>::Type >::template
        Apply<Sequence>
{
};


template<typename Iterator>
struct Next;
//{
//    typedef typename Iterator::Next Type;
//};

template<typename Iterator>
struct Prior;
//{
//    typedef typename Iterator::Prior Type;
//};


template< typename First, typename Last>
struct Distance;

template< typename Tag >
struct AdvanceImpl;

template< typename Iter, int N>
struct Advance : AdvanceImpl < typename Tag<Iter>::Type >::template
        Apply<Iter, N>
{

};


}

#endif // ITER_H
