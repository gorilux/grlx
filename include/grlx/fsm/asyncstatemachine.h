#pragma once

#include <memory>
#include <functional>
#include <utility>
#include <thread>
#include <memory>
#include <future>
#include <condition_variable>
#include <grlx/async/blockingqueue.h>
#include <grlx/fsm/statemachine.h>
#include <grlx/async/threadpool.h>


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
          running(true),
          thread(std::bind(&AsyncStateMachine::Run, this))

    {

    }
    virtual ~AsyncStateMachine()
    {
        {
            std::unique_lock<std::mutex> lock(sync);
            running = false;
            eventQueue.clear();
        }

        cv.notify_all();
        thread.join();
    }
    template<typename EventT>
    void IncomingEvent( EventT const& event)
    {
        eventQueue.push(std::bind(&AsyncStateMachine::template SynchronizedProcessEvent<EventT>,this, event ));
    }

private:
    void Run()
    {
        this->Start();
        while(true)
        {
            ProcessEventAsyncFunc func;
            {
                std::unique_lock<std::mutex> lock(sync);

                cv.wait(lock, [this]() ->bool { return !this->running || !this->eventQueue.empty(); });
                if(!running && eventQueue.empty()){
                    return;
                }
                func = std::move( eventQueue.top() );
                eventQueue.pop();
            }
            func();

        }
    }

private:
    std::mutex sync;
    std::condition_variable cv;
    grlx::async::FifoScheduler<ProcessEventAsyncFunc> eventQueue;
    bool running;
    std::thread thread;



};



} // namespace fsm
} // namespace grlx


