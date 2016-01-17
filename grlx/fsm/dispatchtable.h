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

#ifndef GRLX_FSM_DISPATCHTABLE_H
#define GRLX_FSM_DISPATCHTABLE_H


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
        //ForEach<typename GetStateList< Stt >::Type >( InitCellDefault<Event>(this, entries) );

//        ForEach<typename SelectRowsWhereEvent< Stt, Any >::Result >( InitCell<Event>(this, entries));

//        ForEach< Stt >( InitCell<Event>(this, entries));

        //ForEach<typename SelectRowsWhereEvent< Stt, Event >::Result >( InitCell<Event>(this, entries));        

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




#endif
