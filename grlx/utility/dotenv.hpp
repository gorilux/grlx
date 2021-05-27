#pragma once

#include <string>

namespace grlx {
namespace utils {
void        dotenv();
std::string get_env(std::string const& key);

} // namespace utils

} // namespace grlx