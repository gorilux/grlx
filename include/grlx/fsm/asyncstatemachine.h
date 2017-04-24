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
#ifndef GRLX_FSM_ASYNCSTATEMACHINE_H
#define GRLX_FSM_ASYNCSTATEMACHINE_H

#include <memory>
#include <functional>
#include <utility>
#include <thread>
#include <future>
#include <grlx/async/blockingqueue.h>
#include <grlx/fsm/statemachine.h>


namespace grlx
{
namespace fsm
{


template<typename FSMDef, typename DerivedType = None>
class SyncStateMachine : public StateMachine< FSMDef, DerivedType >
{

    typedef StateMachine< FSMDef, DerivedType > _BaseType;
    typedef SyncStateMachine SelfType;


public:

    template<typename ...TArgs>
    SyncStateMachine(TArgs... args)
        : _BaseType(args...)
    {

    }
    virtual ~SyncStateMachine(){ }

    template<typename EventT>
    void IncomingEvent( EventT const& event)
    {
        SynchronizedProcessEvent( event );
    }
    template<typename EventT>
    HandleStatus::Type SynchronizedProcessEvent( EventT const& event )
    {
        std::unique_lock<std::recursive_mutex> lock(sync);
        return this->ProcessEvent(event);
    }
private:
    std::recursive_mutex sync;

};



template< typename FSMDef, typename DerivedType = None>
class AsyncStateMachine : public SyncStateMachine <FSMDef, DerivedType>
{

    using _BaseType = SyncStateMachine< FSMDef, DerivedType >;
    using SelfType  = AsyncStateMachine;
    using ProcessEventAsyncFunc = std::function< void() >;
    using EventQueueType = grlx::async::BlockingQueue<ProcessEventAsyncFunc>;


public:
    template<typename ...TArgs>
    AsyncStateMachine(TArgs... args)
        : _BaseType(args...),
          eventQueue(new EventQueueType() ),
          thread(std::bind(&AsyncStateMachine::Run, this))

    {

    }
    virtual ~AsyncStateMachine()
    {
        eventQueue.reset();

        thread.join();
    }
    template<typename EventT>
    void IncomingEvent( EventT const& event)
    {
        eventQueue->pushBack(std::bind(&AsyncStateMachine::template SynchronizedProcessEvent<EventT>,this, event ));
    }

private:
    void Run()
    {
        this->Start();
        while(true)
        {
            ProcessEventAsyncFunc func;
            if( eventQueue->popFront(func) )
            {
                func();
            }
            else
            {
                break;
            }
        }
    }

private:
    std::unique_ptr<EventQueueType> eventQueue;
    std::thread thread;



};



} // namespace fsm
} // namespace grlx


#endif // GRLX_FSM_ASYNCSTATEMACHINE_H
