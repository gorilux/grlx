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

#ifndef GRLX_TMPL_FOREACH_H
#define GRLX_TMPL_FOREACH_H

#include "tags.h"

namespace grlx
{


namespace impl
{

template<typename Tag>
struct ForEachImpl
{
    template<typename Sequence, typename F>
    struct Apply;

};

template<typename Sequence, typename F>
struct ForEach : ForEachImpl< typename Tag< Sequence >::Type >::template Apply< Sequence, F>
{

};

}

template<typename Sequence, typename F>
inline void ForEach( Sequence& seq, F const& f)
{
    //impl::ForEach(seq, f, Tag<Sequence>::Type());
    impl::ForEach<Sequence, F>::Exec(seq, f);
}

template<typename Sequence, typename F>
inline void ForEach(Sequence const& seq, F const& f)
{
    //impl::ForEach(seq, f, typename Tag<Sequence>::Type);
    impl::ForEach<Sequence, F>::Exec(seq, f);
}


template<typename Sequence, typename F>
inline void ForEach( F const& f, Sequence* = 0)
{
    //impl::ForEach(seq, f, typename Tag<Sequence>::Type);
    impl::ForEach<Sequence, F>::template Exec<Sequence,F>(f);
}


template<typename Sequence, typename F>
inline void ForEach(F& f, Sequence* = 0)
{
    //impl::ForEach(seq, f, typename Tag<Sequence>::Type);
    impl::ForEach<Sequence, F>::template Exec<Sequence,F>(f);
}




}
#endif // FOREACH_H
