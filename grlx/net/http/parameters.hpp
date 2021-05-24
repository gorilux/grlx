#pragma once

#include <string>
#include <vector>

namespace grlx::net::http {
struct Parameter {
  std::string key;
  std::string value;
};

using Parameters = std::vector<Parameter>;
} // namespace grlx::net::http