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
