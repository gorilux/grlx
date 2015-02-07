#include "dealerendpoint.h"

#include <grlx/fsm/statemachine.h>


namespace grlx {
namespace rpc {
namespace ZeroMQ {


using namespace grlx::fsm;

class DealerEndpointFSM : public grlx::fsm::FsmDefinition<DealerEndpointFSM>
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



class DealerEndpoint::FSM : public grlx::fsm::StateMachine< DealerEndpointFSM, DealerEndpoint::FSM >
{
public:
    explicit FSM(grlx::ServiceContainerPtr serviceContainer, DealerEndpoint* q)
        : serviceContainer(serviceContainer)
        , self(q)
    {}
    virtual ~FSM(){}

    grlx::ServiceContainerPtr serviceContainer;
    DealerEndpoint *self;
};


DealerEndpoint::DealerEndpoint(grlx::ServiceContainerPtr serviceContainer)
    : ZMQSocket(zmq::socket_type::dealer, serviceContainer)
      , fsm( new FSM(serviceContainer, this) )
{

}

DealerEndpoint::DealerEndpoint(const std::string& identity, grlx::ServiceContainerPtr serviceContainer)
    : ZMQSocket(zmq::socket_type::dealer, serviceContainer)
      , fsm( new FSM(serviceContainer, this) )
{
    setSockOpt(ZMQ_IDENTITY, identity.c_str(), identity.size());

}


DealerEndpoint::~DealerEndpoint()
{
}


void DealerEndpoint::send(const char* data, size_t size, const TokenType& userToken)
{
    zmq::message_t payload(data,size);
    sendMsg(payload);
}


void DealerEndpoint::onDataAvailable()
{
    ZMQSocket::onDataAvailable();
}



} // namespace ZeroMQ
} // namespace rpc
} // namespace grlx
