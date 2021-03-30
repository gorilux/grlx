
#include <grlx/service/servicecontainer.h>


#define BOOST_TEST_MODULE GRLX_TEST_SERVICECONTAINER
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( servicecontainer_tests )

BOOST_AUTO_TEST_CASE( servicecontainer_add_get_service )
{
    struct MyTestService{};

    auto serviceContainer = grlx::ServiceContainer::globalInstance();

    serviceContainer->addService<MyTestService>([](){
        return std::make_shared<MyTestService>();
    });

    BOOST_CHECK( serviceContainer->get<MyTestService>() );
}

BOOST_AUTO_TEST_SUITE_END()