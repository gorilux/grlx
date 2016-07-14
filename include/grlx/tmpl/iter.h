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
