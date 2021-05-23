#pragma once

#include <grlx/net/http/message.hpp>
#include <grlx/net/http/version.hpp>
#include <grlx/net/url.hpp>

#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/http/verb.hpp>

#include <grlx/net/alias/beast.hpp>

namespace grlx::net::http {


template <typename DynamicBuffer>
class Basic_Request : public Message<true, DynamicBuffer>
{
  using base_t = Message<true, DynamicBuffer>;

public:
  Basic_Request(beast::http::verb verb, url uri);

  url const& uri() const;
  void accept(std::string_view ct);
  void accept(http::Content_Type const& ct);

private:
  url _uri;
};

// =================

template <typename DynamicBuffer>
Basic_Request<DynamicBuffer>::Basic_Request(beast::http::verb verb, url uri)
  : base_t(verb, uri.target(), 11), _uri(std::move(uri))
{ 
  this->set(beast::http::field::host, _uri.host());
  this->set(beast::http::field::user_agent, USER_AGENT);
  this->set(beast::http::field::cache_control, "no-cache");
}

template <typename DynamicBuffer>
url const& Basic_Request<DynamicBuffer>::uri() const
{
  return _uri;
}

template <typename DynamicBuffer>
void Basic_Request<DynamicBuffer>::accept(std::string_view ct)
{
  this->set(beast::http::field::accept, ct);
}

template <typename DynamicBuffer>
void Basic_Request<DynamicBuffer>::accept(http::Content_Type const& ct)
{
  this->set(beast::http::field::accept, ct);
}

using Request = Basic_Request<beast::multi_buffer>;
  
}