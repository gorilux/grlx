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
#include <grlx/tmpl/sequenceops.h>
#include <grlx/tmpl/typelist.h>
#include "fsmtypes.h"

namespace grlx
{
namespace fsm{


struct SourceStateTag;
struct TargetStateTag;
struct EventTag;


// https://groups.google.com/group/comp.lang.c++.moderated/tree/browse_frm/thread/4f7c7a96f9afbe44/c95a7b4c645e449f?pli=1#doc_e5fbc9305539f699
template <typename Type>
class HasMember
{
    class YesType { char m;};
    class NoType { YesType m[2];};
    struct BaseMixin
    {
        void operator()(){}
    };
    struct Base : public Type, public BaseMixin {};
    template <typename T, T t>  class Helper{};
    template <typename U>
    static NoType Deduce(U*, Helper<void (BaseMixin::*)(), &U::operator()>* = 0);
    static YesType Deduce(...);
public:
    static const bool Result = sizeof(YesType) == sizeof(Deduce((Base*)(0)));
};

namespace details
{
template <typename Type>
class VoidExpResult
{};

template <typename Type, typename U>
U const& operator,(U const&, VoidExpResult<Type>);

template <typename Type, typename U>
U& operator,(U&, VoidExpResult<Type>);

template <typename SrcType, typename DestType>
struct CloneConstness
{
    typedef DestType Type;
};
template <typename SrcType, typename DestType>
struct CloneConstness<const SrcType, DestType>
{
    typedef const DestType type;
};

}

template <typename Type, typename CallDetails>
struct IsCallPossible
{
private:
    class YesType {};
    class NoType { YesType m[2]; };
    struct Derived : public Type
    {
        using Type::operator();
        NoType operator()(...) const;
    };
    typedef typename details::CloneConstness<Type, Derived>::Type DerivedType;

    template <typename T, typename DueType>
    struct ReturnvalueCheck
    {
        static YesType Deduce(DueType);
        static NoType Deduce(...);
        static NoType Deduce(NoType);
        static NoType Deduce(details::VoidExpResult<Type>);
    };
    template <typename T>
    struct ReturnvalueCheck<T, void>
    {
        static YesType Deduce(...);
        static NoType Deduce(NoType);
    };
    template <bool Has, typename F>
    struct impl
    {
        static const bool value = false;
    };
    template <typename Arg1, typename R>
    struct impl<true, R(Arg1)>
    {
        static const bool value =
                sizeof(
                    ReturnvalueCheck<Type, R>::Deduce(
                        (((DerivedType*)0)->operator()(*(Arg1*)0),
                         details::VoidExpResult<Type>())
                        )
                    ) == sizeof(YesType);
    };
    // specializations of impl for 2 args, 3 args,..
public:
    static const bool value = impl<HasMember<Type>::Result,CallDetails>::value;
};


template<typename StateType>
struct IsCompositeState
{

    typedef char NoType[1];
    typedef char YesType[2];

    template<typename S>
    static YesType &Check(S*, typename S::CompositeTag* = 0 );

    template<typename S>
    static NoType &Check( ... );

    static const bool value = (sizeof(YesType) == sizeof( Check<StateType>((StateType*)0) ) );
};

template<typename T>
struct IsInitialStateDefined
{

    typedef char NoType[1];
    typedef char YesType[2];

    template<typename S>
    static YesType &Check(S*, typename S::InitialState* = 0 );

    template<typename S>
    static NoType &Check( ... );

    static constexpr bool value = (sizeof(YesType) == sizeof( Check<T>((T*)0) ) );
};

template<typename StateType>
struct HasDeferedEventList
{
    typedef char NoType[1];
    typedef char YesType[2];

    template<typename S>
    static YesType &Check(S*, typename S::DeferedEvents* = 0 );

    template<typename S>
    static NoType &Check( ... );

    static const bool value = (sizeof(YesType) == sizeof( Check<StateType>((StateType*)0) ) );
};

template<typename StateType, typename Event>
struct DeferedEventListContains
{
    static constexpr bool value =  Contains<  Event, typename StateType::DeferedEvents>::value;
};

template<typename StateType, typename Event>
struct HasDeferedEvent
{
    static constexpr bool value = std::conditional<
                                                    HasDeferedEventList< StateType >::value,
                                                    DeferedEventListContains<StateType,Event> ,
                                                    std::false_type >::type::value;
};


template<typename StateType, typename Event, typename FSM>
struct HasOnEntry
{

    typedef char YesType[1];
    typedef char NoType[2];
    struct BaseMixin
    {
        template<typename E, typename F>
        void OnEntry( E const&, F& )
        {}
    };
    struct Base : public StateType, public BaseMixin {};
    template <typename T, T t>  class Helper{};
    template <typename U>
    static NoType &Deduce(U*, Helper<void (BaseMixin::*)(Event const&, FSM&), &U::OnEntry>* = 0);
    static YesType &Deduce(...);
    static constexpr bool value = sizeof(YesType) == sizeof(Deduce((Base*)(0)));

};

template<typename StateType, typename Event, typename FSM>
struct HasOnExit
{
    typedef char YesType[1];
    typedef char NoType[2];
    struct BaseMixin
    {
        template<typename E, typename F>
        void OnExit( E const&, F& )
        {}
    };
    struct Base : public StateType, public BaseMixin {};
    template <typename T, T t>  class Helper{};
    template <typename U>
    static NoType &Deduce(U*, Helper<void (BaseMixin::*)(Event const&, FSM&), &U::OnExit>* = 0);
    static YesType &Deduce(...);


    static constexpr bool value = sizeof(YesType) == sizeof(Deduce((Base*)(0)));

};

template<typename StateType, typename Event, typename FSM>
struct HasOnEvent
{
    typedef char YesType[1];
    typedef char NoType[2];
    struct BaseMixin
    {
        template<typename E, typename F>
        void OnEvent( E const&, F& )
        {}
    };
    struct Base : public StateType, public BaseMixin {};
    template <typename T, T t>  class Helper{};
    template <typename U>
    static NoType &Deduce(U*, Helper<void (BaseMixin::*)(Event const&, FSM&), &U::OnEvent>* = 0);
    static YesType &Deduce(...);


    static constexpr bool value = sizeof(YesType) == sizeof(Deduce((Base*)(0)));

};

template<typename StateType, typename Event, typename FSM>
struct HasGuard
{

    typedef char YesType[1];
    typedef char NoType[2];
    struct BaseMixin
    {
        template<typename E, typename F>
        bool Guard( E const&, F& )
        { return false; }
    };
    struct Base : public StateType, public BaseMixin {};
    template <typename T, T t>  class Helper{};
    template <typename U>
    static NoType &Deduce(U*, Helper<bool (BaseMixin::*)(Event const&, FSM&), &U::Guard>* = 0);
    static YesType &Deduce(...);


    static constexpr bool value = sizeof(YesType) == sizeof(Deduce((Base*)(0)));
};


//template<typename StateType, typename Event, typename FSM>
//struct HasAction
//{
//    typedef char YesType[1];
//    typedef char NoType[2];
//    struct BaseMixin
//    {
//        template<typename E, typename F>
//        void Action( E const&, F& )
//        {}
//    };
//    struct Base : public StateType, public BaseMixin {};
//    template <typename T, T t>  class Helper{};
//    template <typename U>
//    static NoType &Deduce(U*, Helper<void (BaseMixin::*)(Event const&, FSM&), &U::Action>* = 0);
//    static YesType &Deduce(...);


//    static const bool value = sizeof(YesType) == sizeof(Deduce((Base*)(0)));
//};

template<typename Event>
struct IsAnyEvent : std::is_same<Event, Any>
{

};

template<typename TRANSITION, typename Selector>
struct GetTransitionData;

template<typename TRANSITION>
struct GetTransitionData<TRANSITION, SourceStateTag>
{
  typedef typename TRANSITION::Source Type;
};

template<typename TRANSITION>
struct GetTransitionData<TRANSITION, TargetStateTag>
{
    typedef typename TRANSITION::Target Type;
};

template<typename TRANSITION>
struct GetTransitionData<TRANSITION, EventTag>
{
    typedef typename TRANSITION::Event Type;
};




template< typename StateTransitionTable, template< typename,typename > class GetTransitionDataOp, typename TransitionDataSelector >
struct MakeTransitionDataList
{
    template<typename Stt, typename CurrentList>
    struct Apply
    {
        typedef typename Front<Stt>::Type Row;
        typedef typename PopFront<Stt>::Type Tail;
        typedef typename GetTransitionDataOp< Row, TransitionDataSelector >::Type Data;

        typedef typename std::conditional< Contains<Data, typename CurrentList::Type >::value , CurrentList, typename PushBack<Data, CurrentList>::Type >::type ResultList_;

        typedef typename Apply< Tail , ResultList_>::Result Result;

    };


    template<typename CurrentList>
    struct Apply<TypeList<>::Type, CurrentList>
    {
        // End of the sequence...
        typedef typename CurrentList::Type Result;
    };

    typedef typename Apply< StateTransitionTable, typename TypeList<>::Type >::Result Result;
};

template<typename Stt>
struct MakeSourcesStateList : MakeTransitionDataList<Stt, GetTransitionData, SourceStateTag>
{
};

template<typename Stt>
struct MakeTargetsStateList : MakeTransitionDataList<Stt, GetTransitionData, TargetStateTag>
{
};

template< typename State, bool IsComposite = IsCompositeState< State >::value>
struct GetStt
{
    typedef typename State::SttType Type;
};
template< typename State>
struct GetStt<State, false>
{
    typedef typename TypeList<>::Type Type;
};

template<typename StateTransitionTable>
struct MakeEventTypeList
{

    template<typename Stt, typename CurrentList>
    struct Apply
    {
        typedef typename Front<Stt>::Type Row;
        typedef typename PopFront<Stt>::Type Tail;
        typedef typename Row::Event Event;
        typedef typename Row::Source Source;
        typedef typename Row::Target Target;
        typedef typename std::conditional< Contains<Event, typename CurrentList::Type >::value ,
                                            CurrentList, typename PushBack<Event, CurrentList>::Type >::Type Result_1;


        typedef typename Apply< typename GetStt< Source >::Type, Result_1>::Result Result_2;
        typedef typename Apply< typename GetStt< Target >::Type, Result_2>::Result Result_3;

        typedef typename Apply< Tail , Result_3>::Result Result;

    };


    template<typename CurrentList>
    struct Apply<TypeList<>::Type, CurrentList>
    {
        // End of the sequence...
        typedef typename CurrentList::Type Result;
    };

    typedef typename Apply< StateTransitionTable, typename TypeList<>::Type >::Result Result;


};



template<typename Stt>
struct GetStateList
{
    typedef typename MakeSourcesStateList< Stt >::Result sources;
    typedef typename MakeTargetsStateList< Stt >::Result targets;

    template<typename StateList, typename CurrentList>
    struct Apply
    {
        typedef typename Front<StateList>::Type state;
        typedef typename PopFront<StateList>::Type tail;

        typedef typename std::conditional<
                                            Contains<state, typename CurrentList::Type >::value ,
                                            CurrentList,
                                            typename PushBack<state, CurrentList>::Type >::type resultList_;

        typedef typename Apply< tail , resultList_>::result result;

    };


    template<typename CurrentList>
    struct Apply<TypeList<>::Type, CurrentList>
    {
        // End of the sequence...
        typedef typename CurrentList::Type result;
    };

    typedef typename Apply< targets, sources >::result Type;
};

template<typename Stt>
struct GetCompositeStateList
{
    template<typename Seq, typename State>
    struct SelectCompositeState
    {
        typedef typename std::conditional<
                                            IsCompositeState<State>::value,
                                            typename PushBack<State, Seq>::Type, Seq >::type Type;
    };
    typedef typename GetStateList<Stt>::Type AllStates;
    typedef typename Fold<AllStates, typename TypeList<>::Type, SelectCompositeState >::Type Type;
};

//IF Row starts with ANY then add a row for each from state
template< typename T>
struct IsKleeneClosureType: std::is_same <T, Any>
{};

template<typename RowT>
struct IsKleeneClosureRow: IsKleeneClosureType < typename RowT::Source >
{ };

//template<typename Stt>
//struct GetCompositeStateList
//{

//    typedef typename GetStateList<Stt>::Type AllStates;

//    template<typename StateList, typename CurrentList>
//    struct Apply
//    {
//        typedef typename Front<StateList>::Type State;
//        typedef typename PopFront<StateList>::Type Tail;

//        typedef typename std::conditional< IsCompositeState<State>::value,
//                                   typename PushBack<State, CurrentList>::Type,
//                                   CurrentList >::Type ResultList_;

//        typedef typename Apply< Tail , ResultList_>::Result Result;

//    };


//    template<typename CurrentList>
//    struct Apply<TypeList<>::Type, CurrentList>
//    {
//        // End of the sequence...
//        typedef typename CurrentList::Type result;
//    };

//    typedef typename Apply< AllStates, typename TypeList<>::Type >::Result Type;
//};

template<typename Stt, typename StateType>
struct GetStateID
{
    typedef typename GetStateList< Stt >::Type stateList;
    static constexpr int value = TypeIndex<StateType, stateList>::value;

};

template<typename Stt, typename EventType>
struct GetEventID
{
    typedef typename MakeEventTypeList< Stt >::Result EventList;
    static const int value = TypeIndex<EventType, EventList>::value;
};






template< typename StateTransitionTable, typename StateType>
struct SelectRowsWhereSource
{
    template<typename Stt, typename ResultList>
    struct Apply
    {
        typedef typename Front<Stt>::Type Row;
        typedef typename PopFront<Stt>::Type Tail;
        typedef typename std::conditional< std::is_same< typename Row::Source , StateType>::value,
                                            typename PushBack<Row, ResultList>::Type,
                                            ResultList
                                        >::type ResultList_;

        typedef typename Apply< Tail , ResultList_>::Result Result;

    };
    template<typename ResultList>
    struct Apply<TypeList<>::Type, ResultList>
    {
        typedef typename ResultList::Type Result;
    };

    typedef typename Apply<StateTransitionTable,typename TypeList<>::Type >::Result Result;
};

template< typename StateTransitionTable, typename StateType>
struct SelectRowsWhereTarget
{
    template<typename Stt, typename ResultList>
    struct Apply
    {
        typedef typename Front<Stt>::Type Row;
        typedef typename PopFront<Stt>::Type Tail;
        typedef typename std::conditional< std::is_same< typename Row::Target , StateType>::value,
                                            typename PushBack<Row, ResultList>::Type,
                                            ResultList
                                        >::type ResultList_;

        typedef typename Apply< Tail , ResultList_>::Result Result;

    };
    template<typename ResultList>
    struct Apply<TypeList<>::Type, ResultList>
    {
        typedef typename ResultList::Type Result;
    };

    typedef typename Apply<StateTransitionTable,typename TypeList<>::Type >::Result Result;
};

template< typename StateTransitionTable, typename Event>
struct SelectRowsWhereEvent
{
    template<typename Stt, typename ResultList>
    struct Apply
    {
        typedef typename Front<Stt>::Type Row;
        typedef typename PopFront<Stt>::Type Tail;
        typedef typename std::conditional< std::is_same< typename Row::Event , Event>::value,
                                            typename PushBack<Row, ResultList>::Type,
                                            ResultList
                                        >::type ResultList_;

        typedef typename Apply< Tail , ResultList_>::Result Result;

    };
    template<typename ResultList>
    struct Apply<TypeList<>::Type, ResultList>
    {
        typedef typename ResultList::Type Result;
    };

    typedef typename Apply< StateTransitionTable, typename TypeList<>::Type >::Result Result;
};

template< typename StateTransitionTable, typename Event>
struct SelectRowsEventBelongsToComposite
{

};

template<typename List1T, typename List2T>
struct AppendList
{
    template<typename C1, typename C2>
    struct Apply
    {
        typedef typename Front<C2>::Type Item;
        typedef typename PopFront<C2>::Type Tail;
        typedef typename PushBack<Item, C1>::Type ResultList;

        typedef typename Apply<ResultList, Tail>::Result Result;
    };

    template<typename C>
    struct Apply<C, TypeList<>::Type>
    {
        typedef typename C::Type Result;
    };

    typedef typename Apply< List1T, List2T >::Result Result;


};




template<typename Stt, typename StateType, typename Event>
struct GetNextState
{
    typedef typename SelectRowsWhereSource<Stt, StateType>::Result SelectedRowsStep1;
    typedef typename Front< typename SelectRowsWhereEvent<SelectedRowsStep1, Event>::Result >::Type Row;
    typedef typename Row::Target Type;
    static const int value = GetStateID< Stt, Type>::value;
};


template<typename T>
struct Wrap
{
    typedef T Type;
};


template<typename TypeListType>
struct ToTuple;

template<typename ...TElements>
struct ToTuple< TypeList<TElements...> >
{
    typedef std::tuple<TElements...> Type;
};


}
}
