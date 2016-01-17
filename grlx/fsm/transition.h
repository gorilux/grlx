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

#ifndef GRLX_FSM_TRANSITION_H
#define GRLX_FSM_TRANSITION_H


#include "fsmtypes.h"


namespace grlx
{
namespace fsm
{

struct NormalTransitionTag;
struct DirectTransitionTag;
struct ActionTransitionTag;

template<typename SOURCE,
         typename EVENT,
         typename TARGET,
         typename ACTION = None,
         typename GUARD = None
         >
struct Transition
{
    typedef SOURCE Source;
    typedef EVENT Event;
    typedef TARGET Target;
    typedef ACTION Action;
    typedef GUARD Guard;

};



template<typename SOURCE, typename TARGET>
struct DirectTransition : Transition<SOURCE, None, TARGET, None, None>
{
    typedef Transition<SOURCE, None, TARGET, None, None> BaseType;
//    typedef typename BaseType::Source Source;
//    typedef typename BaseType::Event  Event;
//    typedef typename BaseType::Target Target;
//    typedef typename BaseType::Action Action;
//    typedef typename BaseType::Guard  Guard;

};



}
}

#endif
