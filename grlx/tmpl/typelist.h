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

#ifndef GRLX_TMPL_TYPELIST_H
#define GRLX_TMPL_TYPELIST_H

#include <type_traits>

#include "tags.h"
#include "iter.h"
#include "sequenceops.h"
#include "foreach.h"
#include "int.h"

namespace grlx
{

struct EmptyType_;
struct TypeListTag;

template<typename TypeListT, int N>
struct TypeListIterator;

template<typename ...TElements>
struct TypeList
{
    typedef TypeList<TElements...> Type;
    typedef typename TypeListIterator<Type, 0>::Type Front;
    using type = TypeList<TElements...>;
};


template<typename T, typename TypeListT>
struct IndexOf;

template<typename T, typename HeadT, typename ...TailT>
struct IndexOf<T, TypeList<HeadT, TailT...> >
{
    static constexpr int next_value = IndexOf<T, TailT...>::value;
    static constexpr int value = next_value >= 0 ? next_value + 1: -1;
};

template<typename T, typename ...TailT>
struct IndexOf<T, TypeList<T, TailT...> >
{
    static constexpr int value = 0;
};

template<typename T>
struct IndexOf< T, TypeList<> >
{
    static constexpr int value = -1;
};


template<typename ...TElements>
struct Tag< TypeList<TElements...> >
{
    typedef TypeListTag Type;
};




struct TypeListIteratorTag
{
    typedef TypeListIteratorTag Type;
};

template<typename TypeListT, int N>
struct TypeListIterator
{
    typedef Int<N> Pos;
    typedef typename TypeAt<TypeListT, Pos>::Type Type;
};





template<>
struct TypeAtImpl< TypeListTag >
{
    template< int Idx, typename TypeListT >
    struct ApplyImpl;


    template<int Idx, typename T, typename ...TElements>
    struct ApplyImpl< Idx, TypeList< T, TElements...> >
    {
        typedef typename ApplyImpl<Idx - 1, TypeList<TElements...> >::Type Type;
    };

    template<typename T, typename ...TElements>
    struct ApplyImpl< 0, TypeList<T, TElements...> >
    {
        typedef T Type;
    };

    template<int Idx>
    struct ApplyImpl< Idx, TypeList<> >
    {
        typedef EmptyType_ Type;
    };

    template<typename TypeListT, typename Idx>
    struct Apply : ApplyImpl<Idx::value, TypeListT>
    {

    };


};

template<typename TypeListT, int N>
struct Next< TypeListIterator<TypeListT, N> >
{
    typedef TypeListIterator<TypeListT, N + 1> Type;
};

template<typename TypeListT, int N>
struct Prior< TypeListIterator<TypeListT, N> >
{
    typedef TypeListIterator<TypeListT, N - 1> Type;
};

template<typename ...TElements>
struct TypeListLength;

template<typename ...TElements>
struct TypeListLength< TypeList<TElements...> >
{
    static const int value = sizeof...(TElements);
};



template<>
struct BeginImpl< TypeListTag >
{
    template<typename TListType>
    struct Apply
    {
        typedef TypeListIterator<TListType, 0> Type;
    };
};

template<>
struct EndImpl< TypeListTag >
{
    template<typename TListType>
    struct Apply
    {
        typedef  TypeListIterator<TListType, TypeListLength<TListType>::value > Type;
    };
};

template<>
struct PopFrontImpl<grlx::TypeListTag>
{


    template<typename T>
    struct ApplyImpl;


    template<typename TListType>
    struct Apply : ApplyImpl<TListType>{};

    template<typename T, typename ...TElements>
    struct ApplyImpl< TypeList<T, TElements...> >
    {
        typedef typename TypeList<TElements...>::Type Type;
    };

};

template<>
struct PopBackImpl<grlx::TypeListTag>
{
    template<typename T>
    struct ApplyImpl;


    template<typename TListType>
    struct Apply : ApplyImpl<TListType>{};

//    template<typename ...TElements, typename T>
//    struct ApplyImpl< TypeList< TElements..., T> >
//    {
//        typedef typename TypeList<TElements...>::Type Type;
//    };
};

template<>
struct PushFrontImpl<grlx::TypeListTag>
{

    template<typename T, typename ...TElements>
    struct ApplyImpl;


    template<typename T, typename ...TElements>
    struct ApplyImpl<T, TypeList<TElements...> >
    {
        typedef typename TypeList<T, TElements...>::Type Type;
    };

    template<typename T, typename TListType>
    struct Apply: ApplyImpl<T, TListType>{};


};

template<>
struct PushBackImpl<grlx::TypeListTag>
{
    template<typename TItem, typename ...TElements>
    struct ApplyImpl;


    template<typename TItem, typename ...TElements>
    struct ApplyImpl<TItem, TypeList<TElements...> >
    {
        typedef typename TypeList<TElements..., TItem>::Type Type;
    };

    template<typename TItem, typename TListType>
    struct Apply: ApplyImpl<TItem, TListType>{};

};

template<>
struct TypeIndexImpl< TypeListTag >
{


    template<int Idx, typename TNeedle, typename THayStack>
    struct ApplyImpl;


    template<typename TNeedle, typename THayStack>
    struct Apply : ApplyImpl<0, TNeedle, THayStack>
    {

    };



    template<int Idx, typename TNeedle, typename TItem, typename ...TElements>
    struct ApplyImpl<Idx, TNeedle, TypeList<TItem, TElements...> >
    {
        static const int value = std::is_same< TNeedle, TItem >::value ?
                                        Idx : ApplyImpl<Idx + 1, TNeedle, TypeList<TElements...> >::value;
    };

    template<int Idx, typename TNeedle>
    struct ApplyImpl<Idx, TNeedle, TypeList<> >
    {
        static const int value = -1;
    };
};

template<>
struct SizeImpl< TypeListTag >
{

    template<typename TListType>
    struct Apply : TypeListLength<TListType> {};

};

//template< int N >
//struct TypeIndexImpl< TypeVectorTag<N> >
//{

//public:

//    template<typename Item, typename Sequence>
//    struct Apply
//    {
//    private:

//        template<typename T, typename C>
//        struct ApplyPriv
//        {
//            //private:
//                typedef typename PopFront<C>::Type tail;

//                static const int aux_v1 = EvalIf<
//                                            IsSame<T,
//                                                typename Front<C>::Type
//                                                    >::value,
//                                                Staticvalue<int,
//                                                    Size<Sequence>::value -
//                                                        Size<C>::value > ,
//                                                Staticvalue<int, ApplyPriv<T,
//                                                    tail>::value > >::value;

//                static const int value = aux_v1;
//        };
//        template<typename T>
//        struct ApplyPriv< T, TypeVector0<> >
//        {
//            static const int value = (-1);

//        };

//    public:

//        static const int value = ApplyPriv<Item,Sequence>::value;


//    };
//};

namespace impl
{
template<>
struct ForEachImpl<TypeListTag>
{
    template<typename V, typename F>
    struct Apply
    {

        template<typename First, typename Last, typename FUNCTOR>
        static void Exec_(First*, Last*, FUNCTOR const&, std::true_type)
        {

        }

        template<typename First, typename Last, typename FUNCTOR, typename IsLast>
        static void Exec_(First*, Last*,FUNCTOR const& f, IsLast)
        {
            typedef typename Next<First>::Type Next;
            typedef typename First::Type Item;

            f(Item());

            Exec_(static_cast<Next*>(0), static_cast<Last*>(0), f, typename std::is_same<Next, Last>::type() );
        }

        template<typename SequenceT, typename FUNCTOR>
        static void Exec(FUNCTOR const& f, SequenceT* = 0)
        {
            typedef typename Begin<SequenceT>::Type First;
            typedef typename End<SequenceT>::Type  Last;
            Exec_(static_cast<First*>(0), static_cast<Last*>(0), f, typename std::is_same<First, Last>::type() );
        }

    };
};



} // end namespace impl



}

#endif // TYPELIST_H
