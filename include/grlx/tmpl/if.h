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

#ifndef GRLX_TMPL_IF_H
#define GRLX_TMPL_IF_H


namespace grlx
{
template<bool C, typename T, typename F>
struct If_
{
    typedef T Type;
};

template<typename T, typename F>
struct If_<false,T,F>
{
    typedef F Type;
};


template<bool C, typename  T, typename F>
struct EvalIf: std::conditional<C,T,F> {};

}

#endif
