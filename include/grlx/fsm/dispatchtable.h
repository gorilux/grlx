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



#include <type_traits>

#include <grlx/tmpl/typelist.h>

#include "fsmtypes.h"
#include "transition.h"
#include "utilities.h"

namespace grlx
{

namespace fsm
{

template<typename FSM, typename Stt, typename Event>
struct DispatchTable
{

private:
    static constexpr int StateCount = Size< typename GetStateList< Stt >::Type >::value;
    typedef ::grlx::fsm::HandleStatus::Type (*Cell)(FSM&, Event const&, int,int);
    typedef bool (*Guard)(FSM&, Event const&);

    template<typename EventType, typename StateList>
    struct InitCellDefault;

    template<typename EventType, typename StateT, typename ...RemainingStates>
    struct InitCellDefault<EventType, TypeList< StateT, RemainingStates...> > : InitCellDefault<EventType, TypeList<RemainingStates...> >
    {
        typedef InitCellDefault<EventType, TypeList<RemainingStates...> > BaseType;

        template<typename StateType, bool Defer =  HasDeferedEvent< StateType, EventType >::value || HasDeferedEvent< StateType, Any >::value >
        struct Apply
        {
            Apply(Cell* entries)
            {
                Cell execCall =  &FSM::DeferEvent;
                entries[ GetStateID< Stt, StateType>::value ] = execCall;
            }
        };

        template<typename StateType>
        struct Apply<StateType, false>
        {
            Apply(Cell* entries)
            {
                Cell execCall =  &FSM::InternalNoTransition;
                entries[ GetStateID< Stt, StateType>::value ] = execCall;
            }
        };

        InitCellDefault( Cell* entries): BaseType( entries )
        {
            Apply<StateT> a(entries);
        }
    };

    template<typename EventType>
    struct InitCellDefault<EventType, TypeList<> >
    {
        InitCellDefault( Cell* ){}
    };

    template<typename EventType, typename RowsT>
    struct InitCell;

    template<typename EventType, typename RowT, typename ...RowsT>
    struct InitCell<EventType, TypeList< RowT, RowsT...> > : InitCell<EventType, TypeList< RowsT...> >
    {
        typedef InitCell<EventType, TypeList< RowsT...> > BaseType;

        template<typename TEvent,
                 bool Enable = std::is_base_of<typename RowT::Event, TEvent >::value ||
                               std::is_same<typename RowT::Event, Any>::value
                 >
        struct Apply
        {
            Apply(Cell* entries)
            {
                Cell execCall = &RowT::Execute;
                entries[ GetStateID< Stt, typename RowT::Source >::value ] = execCall;
            }
        };

        template<typename TEvent>
        struct Apply<TEvent, false>
        {
            Apply(Cell*){}
        };

        InitCell(Cell* entries_)
            : BaseType(entries_)
        {
            Apply<EventType> a(entries_);
        }
    };

    template<typename EventType>
    struct InitCell<EventType, TypeList<> >
    {
        InitCell(Cell*)
        {}
    };

public:

    DispatchTable()
    {
        InitCellDefault<Event, typename GetStateList< Stt >::Type >( this->entries );
        InitCell< Event, typename SelectRowsWhereEvent< Stt, Any >::Result >( this->entries );
        InitCell< Event, Stt >( this->entries );
    }
    HandleStatus::Type Exec(FSM& fsm, Event const& ev, int region, int state) const
    {
        return entries[state](fsm, ev, region, state);
    }

    static const DispatchTable Instance;
private:
    Cell entries[StateCount];

};

template<typename FSM, typename Stt>
struct CreateDispatchTable
{
    typedef typename MakeEventTypeList<Stt>::Result EventList;

    template<typename EvList, typename TableList>
    struct Apply
    {
        typedef typename Front<EvList>::Type Event;
        typedef typename PopFront<EvList>::Type Tail;
        typedef DispatchTable<FSM, Stt, Event> Table;

        typedef typename PushBack<Table, TableList>::Type ResultList;
        typedef typename Apply<Tail, ResultList>::Result Result;


    };
    template<typename TableList>
    struct Apply<TypeList<>::Type, TableList>
    {
        typedef typename TableList::Type Result;
    };

    typedef typename Apply<EventList, TypeList<>::Type>::Result Result;
};





}
}
