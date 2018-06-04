#pragma once

#include <memory>

#include <grlx/service/servicecontainer.h>
#include <grlx/rpc/transport/zeromq.h>


namespace grlx {
namespace rpc {
namespace ZeroMQ {


class RouterEndpoint : public ZMQSocket
{
public:
    RouterEndpoint(grlx::ServiceContainerPtr serviceContainer);
    virtual ~RouterEndpoint();

    void send(const char* data, size_t size, TokenType const& userToken);

    // ZMQSocket interface
protected:
    void onDataAvailable() override;

private:
    class FSM;
    friend class FSM;
    std::unique_ptr<FSM> fsm;

};

} // namespace ZeroMQ
} // namespace rpc
} // namespace grlx

