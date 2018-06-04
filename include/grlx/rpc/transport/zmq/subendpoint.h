#pragma once

#include <memory>

#include <grlx/service/servicecontainer.h>
#include <grlx/rpc/transport/zeromq.h>


namespace grlx {
namespace rpc {
namespace ZeroMQ {


class SubEndpoint : public ZMQSocket
{
public:
    SubEndpoint(grlx::ServiceContainerPtr serviceContainer);
    virtual ~SubEndpoint();

    void send(const char* data, size_t size, TokenType const& userToken);


private:
    class FSM;
    friend class FSM;
    std::unique_ptr<FSM> fsm;

};

} // namespace ZeroMQ
} // namespace rpc
} // namespace grlx

