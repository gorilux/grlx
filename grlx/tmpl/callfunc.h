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


#include <functional>
#include <tuple>

namespace grlx {

template<typename S>
struct Signature;

template<typename R, typename... Args>
struct Signature<R(Args...)>
{
    using ReturnType = R;
    using ArgumentType = std::tuple<typename std::decay<Args>::type ... >;
};

template<typename R, typename... Args>
struct Signature<R(Args...) const>
{
    using ReturnType = R;
    using ArgumentType = std::tuple<typename std::decay<Args>::type ... >;
};

template<typename R, typename C, typename... Args>
struct Signature<R(C::*)(Args...)>
{
    using ReturnType = R;
    using ArgumentType = std::tuple<typename std::decay<Args>::type ... >;
};

template<typename R, typename C, typename... Args>
struct Signature<R(C::*)(Args...) const>
{
    using ReturnType = R;
    using ArgumentType = std::tuple<typename std::decay<Args>::type ... >;
};


template<typename T>
struct DeduceLambdaSignature
{
    using type = void;
};

template<typename Ret, typename Class, typename... Args>
struct DeduceLambdaSignature<Ret(Class::*)(Args...) const>
{
    using type = std::function<Ret(Args...)>;
};


template<int...> struct IndexTuple{};

template<int I, typename IndexTuple, typename... Types>
struct MakeIndexesImpl;

template<int I, int... Indexes, typename T, typename ... Types>
struct MakeIndexesImpl<I, IndexTuple<Indexes...>, T, Types...>
{
    typedef typename MakeIndexesImpl<I + 1, IndexTuple<Indexes..., I>, Types...>::type type;
};

template<int I, int... Indexes>
struct MakeIndexesImpl<I, IndexTuple<Indexes...> >
{
    typedef IndexTuple<Indexes...> type;
};

template<typename ... Types>
struct MakeIndexes : MakeIndexesImpl<0, IndexTuple<>, Types...>
{};


template<typename R, typename ...FArgs, typename...TArgs, int ...Indexes>
R callFuncHelper(std::function<R(FArgs...)>& func, IndexTuple< Indexes... > ,std::tuple<TArgs...>&& args)
{
    return func(std::forward<TArgs>(std::get<Indexes>(args))...);
}

template<typename R, typename ...FArgs, typename...TArgs>
R callFunc(std::function<R(FArgs...)>& func, std::tuple<TArgs...>& args)
{
    return callFuncHelper(func, typename MakeIndexes<FArgs...>::type(), std::forward<std::tuple<TArgs...>>(args));
}


template<typename R, typename Class, typename ...FArgs, typename...TArgs, int ...Indexes>
R callMemFuncHelper(R(Class::*mem)(FArgs...) const, Class* obj, IndexTuple< Indexes... > ,const std::tuple<TArgs...>& args)
{
    return (obj->*mem)(std::get<Indexes>(args)...);
}

template<typename R, typename Class, typename ...FArgs, typename...TArgs>
R callMemFunc(R(Class::*mem)(FArgs...) const, Class* obj, const std::tuple<TArgs...>& args)
{
    return callMemFuncHelper(mem, obj, typename MakeIndexes<FArgs...>::type(), args);
}


}

