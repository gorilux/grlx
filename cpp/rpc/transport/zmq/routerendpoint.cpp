#include "routerendpoint.h"

#include <grlx/fsm/statemachine.h>


namespace grlx {
namespace rpc {
namespace ZeroMQ {


using namespace grlx::fsm;

class RouterEndpointFSM : public grlx::fsm::FsmDefinition<RouterEndpointFSM>
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



class RouterEndpoint::FSM : public grlx::fsm::StateMachine< RouterEndpointFSM, RouterEndpoint::FSM >
{
public:
    explicit FSM(grlx::ServiceContainerPtr serviceContainer, RouterEndpoint* q)
        : serviceContainer(serviceContainer)
        , self(q)
    {}
    virtual ~FSM(){}

    std::unordered_map<size_t,zmq::message_t> identities;
    grlx::ServiceContainerPtr serviceContainer;
    RouterEndpoint *self;
};


RouterEndpoint::RouterEndpoint(grlx::ServiceContainerPtr serviceContainer)
    :  ZMQSocket(zmq::socket_type::router, serviceContainer)
    , fsm( new FSM(serviceContainer, this) )
{

}

RouterEndpoint::~RouterEndpoint()
{
}

void RouterEndpoint::send(const char* data, size_t size, const TokenType& userToken)
{
    zmq::message_t payload(data, size);

    auto id = std::any_cast<std::size_t>(userToken);
    auto itr = fsm->identities.find(id);

    if(itr != fsm->identities.end())
    {
        sendMsg(itr->second, ZMQ_SNDMORE);
        sendMsg(payload,0);
    }
    else
    {
        for(auto& elem: fsm->identities)
        {
            sendMsg(elem.second, ZMQ_SNDMORE);
            sendMsg(payload,0);
        }
    }




}


void RouterEndpoint::onDataAvailable()
{
    zmq::message_t identity;

    if(receiveMsg(&identity, ZMQ_DONTWAIT))
    {
        std::hash<zmq::message_t> hasher;
        auto idx = hasher(identity);
        TokenType userToken = idx;
        fsm->identities[idx] = std::move(identity);
        zmq::message_t payload;
        receiveMsg(&payload);

        MsgReceived.Emit(static_cast<const char*>(payload.data()), payload.size(), userToken);

    }
    else
    {
        // TODO:: Handle ZMQReceive error
    }



}


} // namespace ZeroMQ
} // namespace rpc
} // namespace grlx
