#pragma once

////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2015 David Salvador Pinheiro
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is David Salvador Pinheiro
///
/// @author David Salvador Pinheiro
/// @author Copyright 2015, David Salvador Pinheiro
////////////////////////////////////////////////////////////////////////////////


#include <memory>
#include <unordered_map>
#include <type_traits>

#include <zmq.hpp>

#include "serviceprovider.h"

namespace grlx {
namespace rpc {
namespace details {

template<typename T>
class IsDefaultConstructible
{

    typedef char yes;
    typedef struct { char arr[2]; } no;

    template<typename U>
    static decltype(U(), yes()) test(int);

    template<typename>
    static no test(...);

public:

    static const bool value = sizeof(test<T>(0)) == sizeof(yes);
};
}


template<typename EncoderType>
class Server : public ServiceProvider< EncoderType >
{
public:

    Server(grlx::ServiceContainerPtr serviceContainer)
        :serviceContainer(serviceContainer)
    {
        if( !serviceContainer->hasService<zmq::context_t>() )
        {
            serviceContainer->addService<zmq::context_t>([]()
            {
                return std::make_shared<zmq::context_t>(std::thread::hardware_concurrency());
            });
        }
    }

    Server()
        :Server(ServiceContainerFactory::create())
    {
    }


    virtual ~Server()
    {
    }


    void bind(std::string const& addr)
    {

    }

    void disconnect()
    {
    }



private:
    grlx::ServiceContainerPtr serviceContainer;


};

} // namespace rpc
} // namespace grlx
