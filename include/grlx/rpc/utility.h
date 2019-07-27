#pragma once

#include <type_traits>
#include <tuple>
#include <algorithm>
#include <functional>


namespace grlx
{


namespace rpc
{


template<template<typename> class EncoderType,  typename TupleT, int Size = std::tuple_size<TupleT>::value >
struct TupleEncoder;


template< template<typename> class EncoderType, typename TupleT, int Size>
struct TupleEncoder
{
    template<typename Handler>
    static void encode(TupleT const& tuple, Handler& handler)
    {
        typedef typename std::remove_reference<typename std::tuple_element<Size -1, TupleT>::type>::type T;

        TupleEncoder<EncoderType, TupleT, Size - 1>::encode(tuple, handler);

        EncoderType<typename std::remove_const<T>::type >::encode(std::get<Size -1 >(tuple), handler );

    }

    template<typename Handler>
    static bool decode(TupleT& tuple, Handler& handler)
    {
        typedef typename std::remove_reference<typename std::tuple_element<Size -1, TupleT>::type>::type T;

        bool ret = TupleEncoder<EncoderType, TupleT, Size - 1>::decode(tuple, handler);

        ret &= EncoderType<typename std::remove_const<T>::type >::decode(std::get<Size -1 >(tuple), handler );
        return ret;

    }
};

template<template<typename> class EncoderType, typename TupleT>
struct TupleEncoder<EncoderType,TupleT, 0>
{
    template<typename Handler>
    static void encode(TupleT const&, Handler&)
    {

    }

    template<typename Handler>
    static bool decode(TupleT&, Handler&)
    {
        return true;
    }
};


template<typename T>
struct RemoveClass { };
template<typename C, typename R, typename... A>
struct RemoveClass<R(C::*)(A...)> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct RemoveClass<R(C::*)(A...) const> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct RemoveClass<R(C::*)(A...) volatile> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct RemoveClass<R(C::*)(A...) const volatile> { using type = R(A...); };

template<typename T>
struct GetSignatureImpl {
    //using type = typename RemoveClass<decltype(&std::remove_reference<T>::type::operator())>::type;
    using type = typename RemoveClass<typename std::remove_reference<T>::type>::type;
};

template<typename R, typename... A>
struct GetSignatureImpl<R(A...)> { using type = R(A...); };
template<typename R, typename... A>
struct GetSignatureImpl<R(&)(A...)> { using type = R(A...); };
template<typename R, typename... A>
struct GetSignatureImpl<R(*)(A...)> { using type = R(A...); };

template<typename T>
using GetSignature = typename GetSignatureImpl<T>::type;

template<typename F> using MakeFunctionType = std::function<GetSignature<F>>;

template<typename F> MakeFunctionType<F> MakeFunction(F &&f) {
    return MakeFunctionType<F>(std::forward<F>(f));
}


namespace Details
{

template<bool Cond, typename TrueType, typename FalseType>
using SelectIf = std::conditional<Cond, TrueType, FalseType>;


namespace Convert
{

template<typename InputIterator, typename OutputIterator>
void toHex(InputIterator begin, InputIterator end, OutputIterator result, bool lowerCase = false)
{
    static const auto hexDigits = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

    for(InputIterator itr = begin; itr != end; ++itr )
    {
        char c = hexDigits[((*itr) & 0xF0) >> 4];
        *(result++) = c;
        c = hexDigits[(*itr) & 0x0F];
        *(result++) = c;
    }
}

template<typename InputIterator, typename OutputIterator>
void fromHex(InputIterator begin, InputIterator end, OutputIterator result, bool lowerCase = false)
{
    constexpr char decLookupTable[] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // gap before first hex digit
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,2,3,4,5,6,7,8,9,       // 0123456789
        0,0,0,0,0,0,0,             // :;<=>?@ (gap)
        10,11,12,13,14,15,         // ABCDEF
        0,0,0,0,0,0,0,0,0,0,0,0,0, // GHIJKLMNOPQRS (gap)
        0,0,0,0,0,0,0,0,0,0,0,0,0, // TUVWXYZ[/]^_` (gap)
        10,11,12,13,14,15          // abcdef
    };

    for(InputIterator itr = begin; itr != end; )
    {
        int d = decLookupTable[*itr++] << 4;
        d |= decLookupTable[*itr++];
        *(result++) = static_cast<char>(d);
    }
}


} // namespace Convert

}

}
}
