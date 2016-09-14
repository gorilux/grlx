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

#ifndef GRLX_TMPLCALLFUNC_H
#define GRLX_TMPLCALLFUNC_H

#include <functional>

namespace grlx {


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


}

#endif // GRLX_TMPLCALLFUNC_H
