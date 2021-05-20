#pragma once

#include <nlohmann/json.hpp>
#include <skyr/url.hpp>

#include <cstdint>
#include <string>

namespace grlx::net {

using namespace skyr::v1;
class url : public skyr::url {
  using base_t = skyr::url;

public:
  using string_type = base_t::string_type;
  using skyr::url::url;

  string_type target() const {
    return this->pathname() + this->search() + this->hash();
  }
};

namespace url_literals
{
inline auto operator "" _url(const char *str, std::size_t length) {
  return url(std::string_view(str, length));
}
}

} // namespace grlx::net