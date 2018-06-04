#pragma once

#include <memory>

#include <grlx/service/servicecontainer.h>
#include <grlx/rpc/transport/zeromq.h>


namespace grlx {
namespace rpc {
namespace ZeroMQ {


class PubEndpoint : public ZMQSocket
{
public:
    PubEndpoint(grlx::ServiceContainerPtr serviceContainer);
    virtual ~PubEndpoint();

    void send(const char* data, size_t size, const TokenType& userToken);
private:
    class FSM;
    friend class FSM;
    std::unique_ptr<FSM> fsm;

};

} // namespace ZeroMQ
} // namespace rpc
} // namespace grlx

