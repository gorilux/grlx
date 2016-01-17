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

#ifndef GRLX_FSM_STATEMACHINE_H
#define GRLX_FSM_STATEMACHINE_H

#include <type_traits>
#include <algorithm>
#include <functional>
#include <utility>
#include <tuple>
#include <deque>


#include <grlx/tmpl/sequenceops.h>
#include <grlx/tmpl/typelist.h>
#include <grlx/tmpl/if.h>
#include <grlx/tmpl/foreach.h>

#include "fsmtypes.h"
#include "transition.h"
#include "utilities.h"
#include "dispatchtable.h"

namespace grlx
{
namespace fsm
{


template<typename FSM, typename Stt, typename EventT>
const grlx::fsm::DispatchTable<FSM, Stt, EventT> DispatchTable<FSM, Stt, EventT>::Instance;

template<typename Derived, typename BaseType = ::grlx::fsm::None>
class FsmDefinition : public BaseType
{
protected:
    template<typename FSM, typename Event>
    void NoTransition(FSM&, Event const&, int, int)
    {
        //std::cout << "No transition "  << state << std::endl;
    }
};

template<typename Derived, typename BaseType = None>
class FsmConfig : public FsmDefinition < Derived, BaseType >
{

};



template< typename FSMDef, typename Derived = None >
class StateMachine : public FSMDef
{

public:
    typedef StateMachine SelfType;
    typedef FSMDef FSMConfigDef;
    typedef typename FSMConfigDef::Stt Stt;
    typedef typename EvalIf< std::is_same<Derived, None>::value, SelfType, Derived >::type Fsm;
private:

    template<class,class> friend class StateMachine;

    template<typename TRANSITION>
    struct SttRowImpl
    {
        template<typename SOURCE, typename EVENT, typename TARGET, typename ACTION, typename GUARD>
        struct Apply
        {
            typedef SOURCE Source;
            typedef TARGET Target;
            typedef EVENT Event;
            typedef ACTION Action;
            typedef GUARD Guard;

            template<typename FSM, typename EventType>
            static HandleStatus::Type Execute(FSM& fsm, EventType const& ev, int region, int)
            {
                Source& source = fsm.template GetState<Source>();
                Target& target = fsm.template GetState<Target>();

                if(!CheckGuard<Guard>(ev, source, target, fsm))
                {
                    return HandleStatus::GUARDREJECT;
                }

                PerformExit( source,ev, fsm );

                fsm.template SetCurrentState<Target>(region);

                PerformAction<Action>(ev, source, target, fsm);

                PerformEntry<Target>(target, ev, fsm);

                // This performs a transition to the next state if the transition type is direct
                PerformEventLessTransitions<Target>(target, fsm);

                return HandleStatus::SUCCESS;
            }
        };

        template<typename SOURCE, typename EVENT, typename ACTION, typename GUARD>
        struct Apply<SOURCE, EVENT, SOURCE, ACTION, GUARD>
        {
            typedef SOURCE Source;
            typedef SOURCE Target;
            typedef EVENT Event;
            typedef ACTION Action;
            typedef GUARD Guard;

            template<typename FSM, typename EventType>
            static HandleStatus::Type Execute(FSM& fsm, EventType const& ev, int, int)
            {
                Source& source = fsm.template GetState<Source>();

                PerformOnEvent<Source>(source, ev, fsm);

                if(!CheckGuard<Guard>(ev, source, source, fsm))
                {
                    return HandleStatus::GUARDREJECT;
                }                

                PerformAction<Action>(ev, source, source, fsm);

                return HandleStatus::SUCCESS;
            }
        };
        template<typename SOURCE, typename EVENT>
        struct Apply<SOURCE, EVENT, SOURCE, None, None>
        {
            typedef SOURCE Source;
            typedef SOURCE Target;
            typedef EVENT Event;
            typedef None Action;
            typedef None Guard;

            template<typename FSM, typename EventType>
            static HandleStatus::Type Execute(FSM& fsm, EventType const& ev, int, int)
            {
                Source& source = fsm.template GetState<Source>();

                PerformOnEvent<Source>(source, ev, fsm);

                return HandleStatus::SUCCESS;
            }
        };

    };

    template<typename TRANSITION>
    struct SttRow : SttRowImpl< TRANSITION >::template
                            Apply< typename TRANSITION::Source,
                                   typename TRANSITION::Event,
                                   typename TRANSITION::Target,
                                   typename TRANSITION::Action,
                                   typename TRANSITION::Guard
                            >

    {
    };   


    template<typename StateT, typename EventT>
    struct SttFRow
    {
        typedef StateT Source;
        typedef StateT Target;
        typedef EventT Event;
        //typedef None Action;
        //typedef None Guard;

        template<typename FSM, typename EventType>
        static HandleStatus::Type Execute(FSM& fsm, EventType const& ev, int, int)
        {

            HandleStatus::Type status;
            Source& currentState = fsm.template GetState<Source>();

            status = currentState.ForwardEvent(fsm,ev);

            return status;
        }

    };

    template<typename TRANSITION>
    struct CreateSttRow
    {
        typedef SttRow<TRANSITION> Type;
    };


    template<typename StateType, typename Event>
    struct CreateSttFRow
    {
        typedef SttFRow<StateType, Event> Type;
    };

    template<typename STT>
    struct CreateStt
    {
        template<typename SourceList, typename ResultList>
        struct Apply
        {
            typedef typename Front<SourceList>::Type Row;
            typedef typename CreateSttRow< Row >::Type TRow;
            typedef typename PopFront<SourceList>::Type Tail;
            typedef typename Apply< Tail, typename PushBack< TRow, ResultList>::Type >::Result Result;

        };
        template<typename ResultList>
        struct Apply<TypeList<>::Type, ResultList>
        {
            typedef typename ResultList::Type Result;
        };

        typedef typename Apply<STT, TypeList<>::Type >::Result Result;
    };

public:
    typedef  typename CreateStt<Stt>::Result InternalStt;


    template<typename Composite>
    struct ExtendStt
    {

        // add forward rows for composite states
        template<typename ResultStt, typename CompState>
        struct AddForwardRows
        {
            typedef typename MakeEventTypeList< typename CompState::Stt >::Result AllEvents;

            template<typename SttSeq, typename Event>
            struct AddFRow
            {
                typedef typename PushBack< typename CreateSttFRow< CompState, Event>::Type, SttSeq>::Type Type;
            };

            typedef typename Fold<AllEvents, ResultStt, AddFRow >::Type Type;

        };

        typedef typename Composite::InternalStt RawStt;

        template<typename ResultStt, typename Row>
        struct ReplaceKleeneClosureRows
        {
            typedef typename GetStateList< RawStt >::Type StateList;

            
            template<typename STATES, typename ROW,typename STT>
            struct ReplaceSttRows
            {                                
                typedef typename ROW::Source Source;
                typedef typename ROW::Event Event;
                typedef typename ROW::Target Target;
                typedef typename ROW::Action Action;
                typedef typename ROW::Guard Guard;

                template<typename STT_, typename ReplaceState>
                struct ReplaceTransition
                {
                    typedef Transition<ReplaceState, Event, Target, Action, Guard> TRANSITION;
                    typedef typename EvalIf< IsKleeneClosureType<ReplaceState>::value,
                                             STT_,
                                             typename PushBack< SttRow<TRANSITION> , STT_>::Type >::type Type;

                };
                typedef typename Fold<StateList, STT, ReplaceTransition>::Type Type;

            };

            typedef typename EvalIf< IsKleeneClosureRow<Row>::value,
                                     typename ReplaceSttRows<StateList, Row, ResultStt>::Type,
                                     typename PushBack<Row, ResultStt>::Type >::type Type;



        };

        typedef typename Fold<RawStt, TypeList<>::Type, ReplaceKleeneClosureRows >::Type ExpandedStt;
        typedef typename GetCompositeStateList< ExpandedStt >::Type CompositeStates;
        typedef typename Fold<CompositeStates, ExpandedStt, AddForwardRows>::Type Result;
    };


    template<typename ...InitialStates>
    struct GetRegionCount;

    template<typename ...InitialStates>
    struct GetRegionCount< TypeList<InitialStates...> >
    {
        static constexpr int value = sizeof...(InitialStates);
    };



    template<typename T>
    struct GetInitialStates
    {
        typedef typename PushBack< T, TypeList<> >::Type Type;

    };

    template<typename ...InitialStates>
    struct GetInitialStates< TypeList<InitialStates...> >
    {
        typedef TypeList<InitialStates...> Type;
    };


    using CompleteStt   = typename ExtendStt< SelfType >::Result;
    using StateTypeList = typename GetStateList< CompleteStt >::Type ;
    using InitialStates = typename GetInitialStates<typename FSMDef::InitialState>::Type;
    using RegionCount   = GetRegionCount<InitialStates>;


    template<typename StateType>
    StateType& GetState()
    {
        return std::get< GetStateID< CompleteStt, StateType>::value >( this->states );
    }
    template<typename StateType>
    void SetCurrentState(int region)
    {
        Self()->state[region] = GetStateID< CompleteStt, StateType >::value;
    }

    template<typename StateType, typename EventType, typename FsmType>
    static typename std::enable_if< HasOnEntry< StateType, EventType, FsmType>::value >::type
    PerformEntry( StateType& state, EventType const& ev, FsmType& fsm)
    {
        state.OnEntry(ev,fsm);
    }
    template<typename StateType, typename EventType, typename FsmType>
    static typename std::enable_if<  !HasOnEntry< StateType, EventType, FsmType>::value >::type
    PerformEntry( StateType&, EventType const&, FsmType&)
    {

    }

    template<typename StateType, typename EventType, typename FsmType>
    static typename std::enable_if< HasOnExit< StateType, EventType, FsmType>::value >::type
    PerformExit( StateType& state, EventType const& ev, FsmType& fsm)
    {
        state.OnExit(ev, fsm);
    }
    template<typename StateType, typename EventType, typename FsmType>
    static typename std::enable_if< !HasOnExit< StateType, EventType, FsmType>::value >::type
    PerformExit( StateType&, EventType const&, FsmType&)
    {

    }
    template<typename StateType, typename EventType, typename FsmType>
    static typename std::enable_if< HasOnEvent< StateType, EventType, FsmType>::value >::type
    PerformOnEvent(StateType& state, EventType const& ev, FsmType& fsm)
    {
        state.OnEvent(ev,fsm);
    }


    template<typename StateType, typename EventType, typename FsmType>
    static typename std::enable_if< !HasOnEvent< StateType, EventType, FsmType>::value >::type
    PerformOnEvent( StateType&, EventType const&, FsmType&)
    {

    }
    template<typename StateType, typename FsmType >
    static typename std::enable_if<
    Contains<StateType, typename MakeSourcesStateList< typename SelectRowsWhereEvent< Stt, None >::Result>::Result >::value
    >::type
    PerformEventLessTransitions(StateType&, FsmType& fsm)
    {
        fsm.ProcessEvent(None());
    }
    template<typename StateType, typename FsmType>
    static typename std::enable_if< !Contains<StateType,
    typename MakeSourcesStateList< typename SelectRowsWhereEvent< Stt, None >::Result>::Result >::value
    >::type
    PerformEventLessTransitions(StateType&, FsmType&)
    {

    }

    template< typename GuardT, typename EventT, typename SourceStateT, typename TargetStateT, typename FsmType>
    static typename std::enable_if< !std::is_same< GuardT, None>::value ,bool>::type
    CheckGuard(EventT const& ev, SourceStateT& source, TargetStateT& target, FsmType& fsm)
    {
        GuardT guard;
        return guard(ev, source, target, fsm);
    }

    template< typename GuardT, typename EventT, typename SourceStateT, typename TargetStateT, typename FsmType>
    static typename std::enable_if< std::is_same< GuardT, None>::value ,bool>::type
    CheckGuard(EventT const&, SourceStateT&, TargetStateT&, FsmType&)
    {
        return true;
    }

    template< typename ActionT, typename EventT, typename SourceStateT, typename TargetStateT, typename FsmType>
    static typename std::enable_if< !std::is_same< ActionT, None>::value ,void>::type
    PerformAction(EventT const& ev, SourceStateT& source, TargetStateT& target, FsmType& fsm)
    {
        ActionT action;
        action(ev, source, target, fsm);
    }

    template< typename ActionT, typename EventT, typename SourceStateT, typename TargetStateT, typename FsmType>
    static typename std::enable_if< std::is_same< ActionT, None>::value ,void>::type
    PerformAction(EventT const&, SourceStateT&, TargetStateT&, FsmType&)
    {

    }



    template<typename ParentFsmType, typename EventT>
    HandleStatus::Type ForwardEvent( ParentFsmType&, EventT const& ev)
    {
        return this->ProcessEvent(ev);
    }

    template<typename EventType>
    struct HandlePseudoTransitions
    {
        HandlePseudoTransitions( Fsm* fsm )
            : self(fsm)
        {
        }
        void ProcessEvent()
        {
            self->ProcessEvent(EventType());
        }
    private:
        Fsm* self;
    };

    typedef std::function< HandleStatus::Type ()> DeferedEventFunc;
    typedef std::deque< std::pair<DeferedEventFunc, bool> > DeferEventListType;

    //typedef std::deque< std::pair<DeferedEvent*, bool> > DeferEventListType;


    typedef typename ToTuple<StateTypeList>::Type StatesList;

    template<typename EventT>
    struct ProcessEventHelper
    {
        template<typename FSM>
        static HandleStatus::Type Execute(FSM& fsm, EventT&& event, int region, int state)
        {
            using Table = DispatchTable<FSM, CompleteStt, typename std::remove_const<EventT>::type>;
            return Table::Instance.Exec(fsm, std::forward< typename std::remove_reference<EventT>::type>(event), region, state);
        }
    };

    template<typename EventType, typename StateListT>
    struct StartHelper;

    template<typename EventType, typename S, typename ...StatesT>
    struct StartHelper< EventType, TypeList<S, StatesT...> > : StartHelper<EventType, TypeList< StatesT...> >
    {
        using BaseType = StartHelper<EventType, TypeList< StatesT...> >;
        static constexpr int region = BaseType::region - 1;

        template<typename FsmType>
        StartHelper(EventType const& ev, FsmType& fsm)
            : BaseType(ev, fsm)
        {            
            fsm.state[region] = GetStateID<Stt,S>::value;
            FsmType::PerformEntry(std::get< GetStateID<Stt, S>::value >(fsm.states), ev, fsm );
            //PerformEntry<InitialStates> (std::get< GetStateID<Stt, InitialState>::value >(this->states), ev, *Self() );
            FsmType::PerformEventLessTransitions(std::get< GetStateID<Stt, S>::value >(fsm.states), fsm);
        }
    };

    template<typename EventType>
    struct StartHelper< EventType, TypeList<> >
    {
        static constexpr int region = RegionCount::value;

        template<typename FsmType>
        StartHelper(EventType const&, FsmType&){}
    };


public:

    typedef StateMachine Type;
    typedef int CompositeTag;
    typedef Stt SttType;

    template<typename...Args>
    StateMachine(Args...args)
        : FSMConfigDef(args...)
    {
        Init();
    }



    template<typename Event, typename FSM>
    void OnEntry(Event const &ev, FSM &)
    {
        //std::cout << "COMPOSITE INIT" << std::endl;
        this->Start(ev);
    }

    template<typename Event, typename FSM>
    void OnExit(Event const &, FSM &)
    {

       //std::cout << "COMPOSITE Exit" << std::endl;
    }

    template<typename TEvent>
    void Start(TEvent const& ev)
    {
        StartHelper<TEvent, InitialStates>(ev, *Self());        
    }

    void Start()
    {
        this->Start(FsmInit());
    }

    template<typename EventT>
    HandleStatus::Type ProcessEvent( EventT&& event)
    {
        HandleStatus::Type handleStatus = HandleStatus::FAILURE;

        using EventType = typename std::remove_reference<EventT>::type;        

        for(int region = 0; region < RegionCount::value; ++region)
        {

            if(this->isProcessingEvent[region])
            {
                // Queue event
                QueueEvent(*Self(), event, region);
                continue;
            }

            this->isProcessingEvent[region] = true;

            handleStatus = ProcessEventHelper<EventType>::Execute(*Self(), std::forward<EventType>(event),region, this->state[region]);

            this->isProcessingEvent[region] = false;

            this->ProcessDeferedEvents(handleStatus, region);
        }

        return handleStatus;

    }

    template<typename Event>
    HandleStatus::Type PostEvent( Event const& event)
    {
        return ProcessEvent(event);
    }


    template<typename Event>
    static HandleStatus::Type DeferEvent(Fsm& fsm, Event&& ev, int region, int)
    {
        //std::cout << "Defer event= " << GetEventID<Stt, Event>::value << " stateID= " << fsm.state[0] << std::endl;

        QueueEvent(fsm, ev, region);

        return HandleStatus::DEFERRED;
    }
    template<typename Event>
    static void QueueEvent(Fsm& fsm, Event&& ev, int region)
    {
        auto f = std::bind(&Fsm::template ProcessEvent<Event>, &fsm, std::forward<Event>(ev) );

        fsm.deferList[region].push_back( std::make_pair(std::move(f), true) );
    }

    template<typename Event>
    static HandleStatus::Type InternalNoTransition(Fsm& fsm, Event const& event, int region, int state)
    {
        //std::cout << "NoTransition stateID=" << state << std::endl;
        //static_cast<Derived*>(this)->template NoTransition(fsm, event, state);
        fsm.NoTransition(fsm, event, region, state);
        return HandleStatus::FAILURE;
    }

    template<typename Event>
    static HandleStatus::Type DeliverToComposite(Fsm& fsm, Event const& ev, int, int)
    {
        //std::cout << "PseudoTransition stateID=" << state << std::endl;
        //fsm.ProcessEvent(ev);

        return HandleStatus::SUCCESS;
    }

    void ProcessDeferedEvents(HandleStatus::Type handled, int region)
    {

        if(handled == HandleStatus::SUCCESS )
        {
            typename DeferEventListType::iterator itr;
            for( itr = deferList[region].begin(); itr != deferList[region].end(); itr++)
            {
                itr->second = false;
            }
            if(!deferList[region].empty())
            {
                //DeferedEvent* ev = this->deferList.front().first;

                auto evProcessor = std::move( this->deferList[region].front().first );

                this->deferList[region].pop_front();                
                evProcessor();

            }
        }
        else
        {
            typename DeferEventListType::iterator itr;
            for( itr = deferList[region].begin(); itr != deferList[region].end(); itr++)
            {
                if(itr->second == false)
                    break;
            }
            if( itr != deferList[region].end())
            {                
                auto evProcessor = std::move( this->deferList[region].front().first );
                deferList[region].erase(itr);                
                evProcessor();
            }

        }
    }

    int CurrentState(int region)
    {
        return this->state[region];
    }
private:

    void Init()
    {
        //state[0] = GetStateID<Stt,InitialState>::value;
        std::fill_n(state,sizeof(state), 0);
        //isProcessingEvent = false;
        std::fill_n(isProcessingEvent,sizeof(isProcessingEvent), false);
    }

    Fsm* Self()
    {
        return static_cast<Fsm*>(this);
    }

    template<typename FSM, typename Stt, typename Event>
    friend struct DispatchTable;

    StatesList         states;    
    DeferEventListType deferList[RegionCount::value];
    int                state[RegionCount::value];
    bool               isProcessingEvent[RegionCount::value];

};




}
}


#endif
