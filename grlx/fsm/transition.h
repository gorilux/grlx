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
