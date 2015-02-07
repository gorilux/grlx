#include "pubendpoint.h"

#include <grlx/fsm/statemachine.h>


namespace grlx {
namespace rpc {
namespace ZeroMQ {


using namespace grlx::fsm;

class PubEndpointFSM : public grlx::fsm::FsmDefinition<PubEndpointFSM>
{
public:

    //Events
    struct Done{};
    //States
    struct Init
    {
        template<typename TEvent, typename FSM>
        void OnEntry(TEvent const&, FSM& fsm)
        {
            fsm.PostEvent( Done {} );
        }

    };

    struct Idle
    {
        template<typename TEvent, typename FSM>
        void OnEntry(TEvent const&, FSM&)
        {

        }

    };

    using Stt = grlx::TypeList<

    Transition<Init, Done, Idle>
    >;
    using InitialState = Init;

};



class PubEndpoint::FSM : public grlx::fsm::StateMachine< PubEndpointFSM, PubEndpoint::FSM >
{
public:
    explicit FSM(grlx::ServiceContainerPtr serviceContainer, PubEndpoint* q)
        : serviceContainer(serviceContainer)
        , self(q)
    {}
    virtual ~FSM(){}

    grlx::ServiceContainerPtr serviceContainer;
    PubEndpoint *self;
};


PubEndpoint::PubEndpoint(grlx::ServiceContainerPtr serviceContainer)
    : ZMQSocket(zmq::socket_type::pub, serviceContainer)
      ,fsm( new FSM(serviceContainer, this) )
{

}

PubEndpoint::~PubEndpoint()
{
}

void PubEndpoint::send(const char* data, size_t size, const TokenType& userToken)
{
    zmq::message_t payload(data, size);
    sendMsg(payload,0);
}


} // namespace ZeroMQ
} // namespace rpc
} // namespace grlx
