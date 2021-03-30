

#include <sstream>

#include <grlx/rpc/encoder/binary.h>
#include <grlx/rpc/namespaces.h>
#include <grlx/rpc/servicehost.h>
#include <grlx/rpc/invoker.h>


#define BOOST_TEST_MODULE GRLX_TEST_RPC
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( rpc_tests )

BOOST_AUTO_TEST_CASE( rpc_register_service )
{
    namespace rpc = grlx::rpc;
    namespace asio = grlx::asio;

    asio::io_context ctx;

    //ServiceHost 
    auto service = rpc::ServiceHost<rpc::BinaryEncoder>(ctx);

    service.attach("add", [this](int a, int b) -> int {  return a + b; } );

    std::ostringstream ostr;

    service.handleReq(nullptr, 0);

    

    

    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END()