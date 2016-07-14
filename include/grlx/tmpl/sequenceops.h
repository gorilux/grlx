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

#ifndef GRLX_TMPL_SEQUENCEOPS_H
#define GRLX_TMPL_SEQUENCEOPS_H


#include <type_traits>

#include "iter.h"
#include "tags.h"



namespace grlx
{

template<typename Tag>
struct TypeIndexImpl
{
    template<typename T, typename Sequence>
    struct Apply;
};

template<typename T, typename Sequence>
struct TypeIndex : TypeIndexImpl< typename Tag<Sequence>::Type >::template Apply<T,Sequence>
{
};

template< typename Tag>
struct TypeAtImpl
{
    template<typename Sequence, typename N>
    struct Apply;
};

template<typename Sequence, typename N>
struct TypeAt : TypeAtImpl< typename Tag<Sequence>::Type >::template Apply<Sequence, N>
{
};

template<typename SequenceTAG>
struct SizeImpl;


template<typename Sequence>
struct Size: SizeImpl< typename Tag<Sequence>::Type >::template Apply<Sequence>
{
};


template<typename TAG>
struct PushFrontImpl;


template<typename T, typename Sequence>
struct PushFront: PushFrontImpl< typename Tag<Sequence>::Type >::template Apply<T,Sequence>
{
};


template<typename TAG>
struct PushBackImpl;


template<typename T, typename Sequence>
struct PushBack : PushBackImpl< typename Tag<Sequence>::Type >::template Apply<T,Sequence>
{
};


template<typename TAG>
struct PopFrontImpl;


template<typename Sequence>
struct PopFront : PopFrontImpl< typename Tag<Sequence>::Type >::template Apply<Sequence>
{
};


template<typename TAG>
struct PopBackImpl;


template<typename Sequence>
struct PopBack : PopBackImpl< typename Tag<Sequence>::Type >::template Apply<Sequence>
{
};


template<typename Sequence>
struct Front
{
    typedef typename Sequence::Front Type;
};


template<typename Sequence>
struct Back
{
    typedef typename Sequence::Back Type;
};


template<typename T, typename Sequence>
struct Contains
{
    static const bool value = (TypeIndex<T,Sequence>::value >= 0);
};

template< typename Tag>
struct AtImpl
{
    template<typename Sequence, typename N>
    struct Apply;
};

template< typename Sequence, typename N>
struct At : AtImpl< typename Tag<Sequence>::Type >::template Apply<Sequence, N>
{

};

template< template<class> class F, typename X>
struct Apply
{
    //typedef X Type;
    typedef typename F< X >::Type Type;
};

template< template<class, class> class F, typename X1, typename X2>
struct Apply2
{
    //typedef typename Apply<F,  Type;
    //typedef X1 Type;
    typedef typename F< X1, X2 >::Type Type;
};



template< typename Sequence, typename St, template<class,class> class ForwardOp>
struct Fold
{

    template<typename Iterator, typename Last, typename St_>
    struct Apply
    {
        typedef typename Next<Iterator>::Type Next_;
        typedef typename ForwardOp<St_, typename Iterator::Type>::Type Result_;
        typedef typename Apply<Next_, Last, Result_>::State State;

    };

    template<typename Last, typename St_>
    struct Apply<Last, Last, St_>
    {
        typedef St_ State;
    };


    typedef typename Apply< typename Begin< Sequence >::Type,
                            typename End< Sequence>::Type, St>::State Type;

};


//template<int N>
//struct Int
//{
//    using type = typename std::integral_constant<int, N>::type;
//};

//template< int N, typename Sequence>
//inline typename std::ref< typename TypeAt<Sequence, Int<N> >::Type >::type Ref(Sequence& seq)
//{
//    return At<Sequence, Int<N> >::ValueRef(seq);
//}

//template< int N, typename Sequence>
//inline typename std::cref< typename TypeAt<Sequence, Int<N> >::Type >::type Get(const Sequence& seq)
//{
//    return At<Sequence, Int<N> >::Value(seq);
//}

}
#endif // SEQUENCEOPS_H
