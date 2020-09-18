#pragma once
////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2019 David Salvador Pinheiro
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
#include <boost/asio/spawn.hpp>

#include "encoder/binary.h"

namespace grlx {
namespace rpc {

using namespace boost::asio::ip;

template<typename Stream>
class Connection
{

    using HeaderType = std::tuple<std::size_t, std::size_t>;

public:

    using StreamBuffPtr = std::unique_ptr<boost::asio::streambuf>;

    template<typename ... TArgs>
    Connection(tcp::socket socket, TArgs&& ...args)
        : socket_(std::move(socket), std::forward<TArgs>(args)...)        
    {

    }

    template<typename ... TArgs>
    Connection(boost::asio::io_context& ioContext, TArgs&& ...args)
        : socket_(ioContext, std::forward<TArgs>(args)...)        
    {

    }
    void start()
    {
        boost::asio::spawn(socket_.get_executor(), std::bind(&Connection::reader, std::placeholders::_1));
    }

    void stop()
    {
        //asio::spawn

    }

    //template<typename CompletionToken>
    void sendMsg(StreamBuffPtr&& streamBuff)
    {
        auto headerBuffer = nextBuffer();

        HeaderType header{ 0xf0da, streamBuff->size() };

        std::ostream os(headerBuffer.get());

        BinaryEncoder::encode(header, os);

        std::array<boost::asio::const_buffer, 2> buffers =
        {
           headerBuffer->data(),
           streamBuff->data()
        };

        boost::asio::spawn(socket_.get_executor(), [buffers = std::move(buffers), this](boost::asio::yield_context yield)
        {
            boost::system::error_code ec;
            boost::asio::async_write(socket_, buffers, yield[ec]);
        });
    }

    std::unique_ptr<boost::asio::streambuf> nextBuffer()
    {
        return std::make_unique<boost::asio::streambuf>();
    }
private:
    void reader(boost::asio::yield_context yield)
    {

        boost::system::error_code ec;
        boost::asio::streambuf inputBuffer;

        //asio::transfer_at_least(toReadBytes);
        //asio::transfer_exactly
        boost::asio::async_read(socket_, inputBuffer, boost::asio::transfer_at_least(24), yield[ec]);

        if(ec)
        {
            // launch exception...
            return;
        }

        auto readBytes = inputBuffer.size();

        HeaderType header;
        std::istream is(&inputBuffer);
        BinaryEncoder::decodeType(is, header);

        auto headerSize = inputBuffer.size();






    }

//    /// Handle a completed read of a message header. The handler is passed using
//    /// a tuple since boost::bind seems to have trouble binding a function object
//    /// created using boost::bind as a parameter.
//    template <typename T, typename Handler>
//    void handle_read_header(const boost::system::error_code& e, T& t, boost::tuple<Handler> handler)
//    {
//        if (e)
//        {
//            boost::get<0>(handler)(e);
//        }
//        else
//        {
//            // Determine the length of the serialized data.
//            std::istringstream is(std::string(inbound_header_, header_length));
//            std::size_t inbound_data_size = 0;
//            if (!(is >> std::hex >> inbound_data_size))
//            {
//                // Header doesn't seem to be valid. Inform the caller.
//                boost::system::error_code error(boost::asio::error::invalid_argument);
//                boost::get<0>(handler)(error);
//                return;
//            }

//            // Start an asynchronous call to receive the data.
//            inbound_data_.resize(inbound_data_size);
//            void (connection::*f)(const boost::system::error_code&,T&, boost::tuple<Handler>) = &connection::handle_read_data<T, Handler>;

//            boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
//                                    boost::bind(f, this,
//                                                boost::asio::placeholders::error, boost::ref(t), handler));
//        }
//    }

private:
    Stream          socket_;    

};

using SslStream     = boost::asio::ssl::stream<tcp::socket>;

class SslConnection : public Connection<SslStream>
{
public:
    SslConnection(boost::asio::io_context& ioContext, boost::asio::ssl::context& sslContext)
        : Connection<SslStream>(ioContext, sslContext)
    {
    }

};



}
}
