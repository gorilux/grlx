#pragma once

#include <optional>

#include <boost/beast/core/multi_buffer.hpp>

#include <grlx/net/http/content_type.hpp>
#include <grlx/net/http/message.hpp>
#include <grlx/net/alias/asio.hpp>
#include <grlx/net/alias/http.hpp>

#include <nlohmann/json.hpp>

namespace grlx::net::http {

template <typename DynamicBuffer>
class basic_response : public message<false, DynamicBuffer> {
public:
  using base_t          = message<false, DynamicBuffer>;
  using base_t::operator=;
  using base_t::base_t;
  bool ok() const
  {
    return to_status_class(this->result()) == beast::http::status_class::successful;
  }
};

using response = basic_response<beast::multi_buffer>;

} // namespace grlx::net::http