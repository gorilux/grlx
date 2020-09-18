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


#include <list>
#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "namespaces.h"

#include "endpoint.h"
#include "connection.h"

namespace grlx
{
namespace rpc
{

using namespace boost::asio::ip;

template<typename TEncoderType, typename Stream>
class Session : public Endpoint<TEncoderType, Connection<Stream> >
{
public:

};



template<typename TEncoderType>
class Server
{
private:

    struct TcpAcceptor
    {

        template<typename CreateSessionCallback>
        TcpAcceptor(boost::asio::io_context& ioContext, tcp::endpoint&& endpoint, CreateSessionCallback callback)
            : acceptor(ioContext, std::move(endpoint))
            , createSessionCallback( callback )
        {
            performAccept();
        }
        void performAccept()
        {
            acceptor.async_accept( [this](const std::error_code& error, tcp::socket socket)
            {
                if (!error)
                {
                    createSessionCallback(std::move(socket));
                }
                performAccept();
            });
        }
        boost::asio::ip::tcp::acceptor acceptor;
        std::function<void(tcp::socket)> createSessionCallback;
    };



public:

    Server(asio::io_context& ioContext)
        : ioContext_(ioContext)
    {
        //sslContext_(asio::ssl::context::sslv23)
    }

    void listen(std::string const& addr, std::uint16_t port)
    {
        auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(addr), port);

        auto acceptor = std::make_unique<TcpAcceptor>(ioContext_, std::move(endpoint), [this](tcp::socket socket)
        {

        });

        acceptors.emplace_back(std::move(acceptor));
    }

    void listenSsl(std::string const& addr, int port)
    {
        auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(addr), port);

        auto acceptor = std::make_unique<TcpAcceptor>(ioContext_, std::move(endpoint), [this](tcp::socket socket)
        {

        });

        acceptors.emplace_back(std::move(acceptor));
    }
    
    
    

    std::shared_ptr<boost::asio::ssl::context> sslContext()
    {
        return sslContext_;
    }


    template<typename NewSessionCallback>
    void setNewSessionCallback(NewSessionCallback cb)
    {

    }

private:
    asio::io_context& ioContext_;
    std::shared_ptr<boost::asio::ssl::context> sslContext_;
    std::list<std::unique_ptr<TcpAcceptor>> acceptors;

};


}
}
