#include "zmqsocket.h"

#include <unordered_set>
#include <grlx/fsm/statemachine.h>
#ifndef ANDROID
#include <glog/logging.h>
#endif


namespace grlx {
namespace rpc {
namespace ZeroMQ {


using namespace grlx::fsm;

class ZMQSocketFSM : public grlx::fsm::FsmDefinition<ZMQSocketFSM>
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



class ZMQSocket::FSM : public grlx::fsm::StateMachine< ZMQSocketFSM, ZMQSocket::FSM >
{
public:
    explicit FSM(grlx::ServiceContainerPtr serviceContainer, ZMQSocket* q)
        : serviceContainer(serviceContainer)
        , self(q)
    {}
    virtual ~FSM(){}

    void init(zmq::socket_type socketType)
    {
        if( !serviceContainer->hasService<zmq::context_t>() )
        {
            serviceContainer->addService<zmq::context_t>([]()
            {
                return std::make_shared<zmq::context_t>(std::thread::hardware_concurrency());
            });
        }
        if( !serviceContainer->hasService< Poller > ())
        {
            serviceContainer->addService<Poller>([this]()
            {
                return std::make_shared<Poller>(serviceContainer);
            });
        }

        auto zmq_ctx = serviceContainer->get<zmq::context_t>();


        zmq_socket.reset( new zmq::socket_t( *zmq_ctx, socketType ));

        attachMonitor(*zmq_socket, ZMQ_EVENT_ALL);


        auto poller = serviceContainer->get<Poller>();

        poller->add(zmq_socket.get(), ZMQ_POLLIN, [this](zmq::socket_t* socket)
        {
            self->onDataAvailable();

        });
    }
    void attachMonitor(zmq::socket_t &socket, int events = ZMQ_EVENT_ALL)
    {
        void* ptr = static_cast<void*>(socket);


        std::string monitorAddr = "inproc://" + std::to_string(reinterpret_cast<std::intptr_t>(ptr))
                                  + ".monitor";

        int rc = zmq_socket_monitor(ptr, monitorAddr.c_str(), events);
        if (rc != 0)
            throw int();


        auto poller = serviceContainer->get<Poller>();

        auto zmq_ctx = serviceContainer->get<zmq::context_t>();

        zmq_monitor_socket.reset( new zmq::socket_t( *zmq_ctx, ZMQ_PAIR ));

        zmq_monitor_socket->connect(monitorAddr);

        poller->add(zmq_monitor_socket.get(), ZMQ_POLLIN, [this](zmq::socket_t* socket)
        {
            zmq::message_t msg;
            socket->recv(&msg);
            zmq_event_t msgEvent;

            auto data = static_cast<const char*>(msg.data());
            memcpy(&msgEvent.event, data, sizeof(uint16_t));
            data += sizeof(uint16_t);
            memcpy(&msgEvent.value, data, sizeof(int32_t));

            zmq::message_t addrMsg;
            socket->recv(&addrMsg);

            const char* str = static_cast<const char*>(addrMsg.data());
            std::string address(str, str + addrMsg.size());

            //#ifdef ZMQ_EVENT_MONITOR_STOPPED
            //            if (msgEvent.event == ZMQ_EVENT_MONITOR_STOPPED)
            //            {
            //                zmq_monitor_socket->close();
            //                return;
            //            }
            //#endif

            switch (msgEvent.event)
            {
                case ZMQ_EVENT_CONNECTED:
//                    if(socketType == zmq::socket_type::sub)
//                    {
//                        zmq_socket->setsockopt( ZMQ_SUBSCRIBE, "", 0);
//                        std::this_thread::sleep_for(std::chrono::milliseconds(500));

//                    }
                    self->onEventConnected(msgEvent, address);
                    break;
                case ZMQ_EVENT_CONNECT_DELAYED:
                    self->onEventConnectDelayed(msgEvent, address);
                    break;
                case ZMQ_EVENT_CONNECT_RETRIED:
                    self->onEventConnectRetried(msgEvent, address);
                    break;
                case ZMQ_EVENT_LISTENING:
                    self->onEventListening(msgEvent, address);
                    break;
                case ZMQ_EVENT_BIND_FAILED:
                    self->onEventBindFailed(msgEvent, address);
                    break;
                case ZMQ_EVENT_ACCEPTED:
//                    if(SocketType == zmq::socket_type::sub)
//                    {
//                        zmq_socket->setsockopt( ZMQ_SUBSCRIBE, "", 0);
//                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
//                    }

                    self->onEventAccepted(msgEvent, address);
                    break;
                case ZMQ_EVENT_ACCEPT_FAILED:
                    self->onEventAcceptFailed(msgEvent, address);
                    break;
                case ZMQ_EVENT_CLOSED:
                    self->onEventClosed(msgEvent, address);
                    break;
                case ZMQ_EVENT_CLOSE_FAILED:
                    self->onEventCloseFailed(msgEvent, address);
                    break;
                case ZMQ_EVENT_DISCONNECTED:
                    self->onEventDisconnected(msgEvent, address);
                    break;
#ifdef ZMQ_BUILD_DRAFT_API
                case ZMQ_EVENT_HANDSHAKE_FAILED:
                    self->on_event_handshake_failed(*event, address);
                    break;
                case ZMQ_EVENT_HANDSHAKE_SUCCEED:
                    self->on_event_handshake_succeed(*event, address);
                    break;
#endif
                default:
                    self->onEventUnknown(msgEvent, address);
                    break;
            }

        });



        self->onMonitorStarted();
    }

    void dettachMonitor()
    {
        if (zmq_monitor_socket)
        {

            auto poller = serviceContainer->get<Poller>();

            zmq_socket_monitor(static_cast<void*>(*zmq_monitor_socket), nullptr, 0);
            poller->remove(zmq_monitor_socket.get());

            zmq_monitor_socket.reset();
        }
    }

    void dispose()
    {

        self->close();

        auto poller = serviceContainer->get<Poller>();

        poller->remove(zmq_socket.get());
        dettachMonitor();



    }


    grlx::ServiceContainerPtr serviceContainer;
    std::shared_ptr<zmq::context_t> zmq_ctx;
    std::unique_ptr<zmq::socket_t> zmq_socket;
    std::unique_ptr<zmq::socket_t> zmq_monitor_socket;

    std::unordered_set<std::string> connectedAddresses;
    std::unordered_set<std::string> bindAddresses;
    ZMQSocket *self;
};


ZMQSocket::ZMQSocket(zmq::socket_type socketType, grlx::ServiceContainerPtr serviceContainer)
    : fsm( new FSM(serviceContainer, this) )
{
    fsm->self = this;
    fsm->serviceContainer = serviceContainer;

    fsm->init(socketType);
}

ZMQSocket::~ZMQSocket()
{
    fsm->dispose();
}

void ZMQSocket::connect(const std::string& addr)
{
    fsm->connectedAddresses.insert(addr);
    fsm->zmq_socket->connect(addr);
}

void ZMQSocket::disconnect(const std::string& addr)
{
    fsm->zmq_socket->disconnect(addr);
    fsm->connectedAddresses.erase(addr);
}

void ZMQSocket::bind(const std::string& addr)
{
    fsm->bindAddresses.insert(addr);
    fsm->zmq_socket->bind(addr);
}

void ZMQSocket::listen(const std::string& addr)
{
    bind(addr);
}

void ZMQSocket::unbind(const std::string& addr)
{
    fsm->zmq_socket->unbind(addr);
    fsm->bindAddresses.erase(addr);
}

void ZMQSocket::send(const char* data, size_t size)
{
//    std::cout << type(*this) << " Snd:";
//    std::cout.write(data, size);
//    std::cout << std::endl;

    zmq::message_t msg(data, size);
    sendMsg(msg, 0);
}

void ZMQSocket::close()
{
    for(auto& addr: fsm->connectedAddresses)
    {
        fsm->zmq_socket->disconnect(addr);
    }
    fsm->connectedAddresses.clear();

    for(auto& addr: fsm->bindAddresses)
    {
        fsm->zmq_socket->unbind(addr);
    }
    fsm->bindAddresses.clear();
}

void ZMQSocket::setSockOpt(int option_, const void* optval_, size_t optvallen_)
{
    fsm->zmq_socket->setsockopt(option_, optval_, optvallen_);
}

void ZMQSocket::getSockOpt(int option_, void* optval_, size_t* optvallen_) const
{
    fsm->zmq_socket->getsockopt(option_, optval_, optvallen_);
}

void ZMQSocket::onMonitorStarted()
{
}

void ZMQSocket::onEventConnected(const zmq_event_t& event_, const std::string& addr_)
{
    //std::cout << __PRETTY_FUNCTION__ << std::endl;
    this->Connected.Emit();
}

void ZMQSocket::onEventConnectDelayed(const zmq_event_t& event_, const std::string& addr_)
{
    this->ConnectDelayed.Emit();
}

void ZMQSocket::onEventConnectRetried(const zmq_event_t& event_, const std::string& addr_)
{
    this->ConnectRetried.Emit();
}

void ZMQSocket::onEventListening(const zmq_event_t& event_, const std::string& addr_)
{
    this->Listening.Emit();
}

void ZMQSocket::onEventBindFailed(const zmq_event_t& event_, const std::string& addr_)
{
    this->BindFailed.Emit();
}

void ZMQSocket::onEventAccepted(const zmq_event_t& event_, const std::string& addr_)
{
    this->Accepted.Emit();
}

void ZMQSocket::onEventAcceptFailed(const zmq_event_t& event_, const std::string& addr_)
{
    this->AcceptFailed.Emit();
}

void ZMQSocket::onEventClosed(const zmq_event_t& event_, const std::string& addr_)
{
    this->Closed.Emit();
}

void ZMQSocket::onEventCloseFailed(const zmq_event_t& event_, const std::string& addr_)
{
    this->CloseFailed.Emit();
}

void ZMQSocket::onEventDisconnected(const zmq_event_t& event_, const std::string& addr_)
{
    this->Disconnected.Emit();
}

void ZMQSocket::onEventHandshakeFailed(const zmq_event_t& event_, const std::string& addr_)
{
    this->HandshakeFailed.Emit();
}

void ZMQSocket::onEventHandshakeSucceed(const zmq_event_t& event_, const std::string& addr_)
{
    this->HandshakeSucceed.Emit();
}

void ZMQSocket::onEventUnknown(const zmq_event_t& event_, const std::string& addr_)
{
    this->Unknown.Emit();
}

void ZMQSocket::onDataAvailable()
{
    zmq::message_t msg;
    if(receiveMsg(&msg, ZMQ_DONTWAIT))
    {
        std::vector<char> buffer(static_cast<const char*>(msg.data()), static_cast<const char*>(msg.data()) + msg.size());
        uint32_t more = 0;
        size_t moresz = sizeof(more);

        while(true)
        {
            fsm->zmq_socket->getsockopt(ZMQ_RCVMORE, &more, &moresz);

            if(more && receiveMsg(&msg, ZMQ_DONTWAIT))
            {
                auto first = static_cast<const char*>(msg.data());
                auto last = static_cast<const char*>(msg.data()) + msg.size();
                buffer.insert(buffer.end(), first, last);
            }
            else
            {
                break;
            }
        }


        MsgReceived.Emit(static_cast<const char*>(buffer.data()), buffer.size(), nullptr);
    }
    else
    {
        // TODO:: Handle ZMQReceive error
    }

}

bool ZMQSocket::receiveMsg(zmq::message_t* msg, int flags)
{
    return fsm->zmq_socket->recv(msg, flags);
}

size_t ZMQSocket::sendMsg(zmq::message_t& msg, int flags)
{
    return fsm->zmq_socket->send(msg, flags);
}


} // namespace ZeroMQ
} // namespace rpc
} // namespace grlx
