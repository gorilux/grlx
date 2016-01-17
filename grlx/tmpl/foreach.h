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
