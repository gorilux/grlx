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
class Basic_Response : public Message<false, DynamicBuffer> {
public:
  using base_t          = Message<false, DynamicBuffer>;
  using base_t::operator=;
  using base_t::base_t;
  bool ok() const
  {
    return to_status_class(this->result()) == beast::http::status_class::successful;
  }
};

using Response = Basic_Response<beast::multi_buffer>;

} // namespace grlx::net::http