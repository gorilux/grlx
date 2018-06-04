#include "subendpoint.h"

#include <grlx/fsm/statemachine.h>


namespace grlx {
namespace rpc {
namespace ZeroMQ {


using namespace grlx::fsm;

class SubEndpointFSM : public grlx::fsm::FsmDefinition<SubEndpointFSM>
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



class SubEndpoint::FSM : public grlx::fsm::StateMachine< SubEndpointFSM, SubEndpoint::FSM >
{
public:
    explicit FSM(grlx::ServiceContainerPtr serviceContainer, SubEndpoint* q)
        : serviceContainer(serviceContainer)
        , self(q)
    {}
    virtual ~FSM(){}

    grlx::ServiceContainerPtr serviceContainer;
    SubEndpoint *self;
};


SubEndpoint::SubEndpoint(grlx::ServiceContainerPtr serviceContainer)
    : ZMQSocket(zmq::socket_type::sub, serviceContainer)
    , fsm( new FSM(serviceContainer, this) )
{
    setSockOpt( ZMQ_SUBSCRIBE, "", 0);

}

SubEndpoint::~SubEndpoint()
{
}

void SubEndpoint::send(const char* data, size_t size, const TokenType& userToken)
{

}


} // namespace ZeroMQ
} // namespace rpc
} // namespace grlx
