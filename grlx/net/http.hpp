#pragma once

#include <grlx/net/http/authorization.hpp>
#include <grlx/net/http/field.hpp>
#include <grlx/net/http/headers.hpp>
#include <grlx/net/http/request.hpp>
#include <grlx/net/http/response.hpp>
#include <grlx/net/url.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

#include <grlx/net/alias/asio.hpp>
#include <grlx/net/alias/beast.hpp>
#include <grlx/net/alias/ssl.hpp>
#include <grlx/net/alias/tcp.hpp>
#include <grlx/tmpl/overloaded.hpp>
#include <grlx/tmpl/unfold.hpp>

#include <filesystem>
#include <fmt/core.h>
#include <map>
#include <memory>
#include <optional>

#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace grlx::net {

using tcp_layer = beast::tcp_stream;
using tls_layer = beast::ssl_stream<tcp_layer>;

using plain_or_tls = std::variant<std::monostate, tcp_layer, tls_layer>;

class Http {
public:

  Http() {}
  Http(std::string base_url){}

  bool is_open() {
    if (std::holds_alternative<tls_layer>(stream)) {
      return tls_stream().next_layer().socket().is_open();
    } else if (std::holds_alternative<tcp_layer>(stream)) {
      return plain_stream().socket().is_open();
    } else {
      return false;
    }
  }

  template <typename... TArgs>
  asio::awaitable<http::response> get(std::string_view target, TArgs... args) {
    return perform_request(beast::http::verb::get, target, std::forward<TArgs>(args)...);
  }
  template <typename... TArgs>
  asio::awaitable<http::response> post(std::string_view target, TArgs... args) {
    return perform_request(beast::http::verb::post, target, std::forward<TArgs>(args)...);
  }
  template <typename... TArgs>
  asio::awaitable<http::response> head(std::string_view target, TArgs... args) {
    return perform_request(beast::http::verb::head, target, std::forward<TArgs>(args)...);
  }
  template <typename... TArgs>
  asio::awaitable<http::response> options(std::string_view target, TArgs... args) {
    return perform_request(beast::http::verb::options, target, std::forward<TArgs>(args)...);
  }
  template <typename... TArgs>
  asio::awaitable<http::response> patch(std::string_view target, TArgs... args) {
    return perform_request(beast::http::verb::patch, target, std::forward<TArgs>(args)...);
  }
  template <typename... TArgs>
  asio::awaitable<http::response> delete_(std::string_view target, TArgs... args) {
    return perform_request(beast::http::verb::delete_, target, std::forward<TArgs>(args)...);
  }
  template <typename... TArgs>
  asio::awaitable<http::response> put(std::string_view target, TArgs... args) {
    return perform_request(beast::http::verb::put, target, std::forward<TArgs>(args)...);
  }

  asio::awaitable<void> disconnect() {
    return std::visit(overloaded{
                   [](std::monostate&) -> asio::awaitable<void> { co_return; },
                   [](tls_layer& stream) -> asio::awaitable<void> {
                     return stream.async_shutdown(asio::use_awaitable);
                   },
                   [](tcp_layer& stream) -> asio::awaitable<void> {
                     beast::error_code ec;
                     stream.socket().lowest_layer().shutdown(tcp::socket::shutdown_send, ec);
                     co_return;
                   }},
               stream);
    
  }

private:
  template <typename... TArgs>
  asio::awaitable<http::response> perform_request(beast::http::verb verb, std::string_view target, TArgs... args) {
    http::response response;

    auto target_url = net::url(target, base_url.value_or(net::url()));
    if (base_url == std::nullopt) {
      base_url = target_url;
    }

    auto apply_parameters = [&](http::request& request) {
      unfold<1>(overloaded{
                    [](auto& target) {},
                    [&request](http::headers fields) {
                      for (auto const& field : fields) {
                        request.insert(field.field, field.field_name, field.value);
                      }
                    },
                },
                args...);
    };

    auto request = http::request(verb, target_url);

    apply_parameters(request);

    auto executor = co_await asio::this_coro::executor;
        
    tcp::resolver resolver(executor);

    auto endpoints = co_await resolver.async_resolve(target_url.hostname(), target_url.port(), asio::use_awaitable);

    if (current_url.protocol() != target_url.protocol() && current_url.host() != target_url.host() && is_open()) {

      co_await disconnect();
    }

    tcp_layer* socket;

    bool is_secure = (target_url.scheme() == "https");

    if (is_secure) {
      stream.emplace<tls_layer>(executor, ssl_ctx);
      // Set SNI Hostname (many hosts need this to handshake successfully)
      // XXX: openssl specificae, abstract this shit please
      if (!SSL_set_tlsext_host_name(tls_stream().native_handle(), target_url.host().data())) {
        boost::system::error_code ec{static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()};
        std::cerr << ec.message() << "\n";
        co_return response;
      }
      socket = &tls_stream().next_layer();
    } else {
      stream.emplace<tcp_layer>(executor);
      socket = &plain_stream();
    }

    //co_await asio::async_connect(*socket, endpoints.begin(), endpoints.end(), asio::use_awaitable);
    co_await socket->async_connect(endpoints, asio::use_awaitable);

    current_url = target_url;

    // negociate ssl connection if applicable
    co_await std::visit(overloaded{
                            [](std::monostate&) -> asio::awaitable<void> { co_return; },
                            [](tls_layer& stream) -> asio::awaitable<void> {
                              return stream.async_handshake(ssl::stream_base::client, asio::use_awaitable);
                            },
                            [](tcp_layer&) -> asio::awaitable<void> { co_return; },
                        },
                        stream);

    // Send the HTTP request to the remote host
    bool follow_next_redirect = false;
    int  number_of_redirects  = max_redirects;
    do {
      // lets add auth headers if any are avaliable
      if (!auth_decorator(request)) {
        std::cerr << "Could not set auth data " << std::endl;
      }

      follow_next_redirect = false;

      response = co_await std::visit(overloaded{
                                         [](std::monostate&) { return asio::awaitable<http::response>(); },
                                         [&request, this](auto& stream) -> asio::awaitable<http::response> {
                                           return rest_call(stream, request);
                                         }},
                                     stream);

      if (auto location_itr = response.find(http::field::location); response.result_int() >= 300 && response.result_int() <= 310 && location_itr != response.end()) {
        auto redirect_url = url(location_itr->value(), request.uri());
        if (redirect_url.host() == request.uri().host()) {
          request              = http::request(verb, redirect_url);
          follow_next_redirect = true;
        } else {
          follow_next_redirect = false;
        }
      }

      if (response.result() == beast::http::status::unauthorized && generate_authentication(response, request)) {
        follow_next_redirect = true;
      }

    } while (follow_next_redirect && --number_of_redirects >= 0);

    co_return response;
  }

  template <typename Stream>
  asio::awaitable<http::response> rest_call(Stream& stream, http::request const& request) {
    beast::flat_buffer buffer;
    http::response     response;

    co_await beast::http::async_write(stream, request, asio::use_awaitable);
    co_await beast::http::async_read(stream, buffer, response, asio::use_awaitable);

    co_return response;
  }
  tls_layer& tls_stream() {
    return std::get<tls_layer>(stream);
  }
  tcp_layer& plain_stream() {
    return std::get<tcp_layer>(stream);
  }
  tcp_layer& get_tcp_layer() {

    return std::visit(overloaded{
                          [](std::monostate&) -> tcp_layer& {
                            assert(!"logic error - stream not open");
                            throw std::logic_error("stream not open");
                          },
                          [](tcp_layer& stream) -> tcp_layer& {
                            return stream;
                          },
                          [](tls_layer& stream) -> tcp_layer& {
                            return stream.next_layer();
                          }},
                      stream);
  }

  bool generate_authentication(http::response const& response, http::request& request) {

    auto& url = request.uri();
    if (url.username().empty() && url.password().empty()) {
      auth_decorator = [](auto&) { return true; };
      return false;
    }
    auto authenticate = response[http::field::www_authenticate];
    if (boost::ifind_first(authenticate, "digest")) {

      auth_decorator = [digest = http::authorization::digest(url.username(), url.password(), authenticate)](http::request& request) mutable {
        if (!digest.generate_authorization(beast::http::to_string(request.method()), request.uri().target(), std::string_view())) {
          return false;
        }
        request.set(digest);
        return true;
      };

    } else {
      auth_decorator = [basic = http::authorization::basic(url.username(), url.password())](http::request& request) mutable {
        request.set(basic);
        return true;
      };
    }
    return true;
  }

private:  
  std::optional<url>                                              base_url;
  net::url                                                        current_url;
  ssl::context                                                    ssl_ctx{ssl::context::tlsv12_client};
  plain_or_tls                                                    stream;
  std::function<bool(http::request&)>                             auth_decorator = [](auto&) { return true; };
  int                                                             max_redirects  = 5;
};

} // namespace grlx::net