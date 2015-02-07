#pragma once

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
