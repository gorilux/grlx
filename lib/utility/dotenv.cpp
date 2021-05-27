#include "dotenv.hpp"

#include <boost/filesystem.hpp>
#include <boost/xpressive/xpressive.hpp>

namespace utils {

using namespace boost::xpressive;

std::string get_env(std::string const& key) {
  const char* val = std::getenv(key.c_str());
  return val == nullptr ? std::string("") : std::string(val);
}

void dotenv() {
  if (boost::filesystem::exists(".env")) {
    std::ifstream env_file(".env");
    for (std::string line; std::getline(env_file, line);) {
      sregex expr = (s1 = +set[range('0', '9') | range('a', 'z') | range('A', 'Z') | '_' | '-']) >> '=' >> (s2 = +print);
      smatch what;
      if (regex_match(line, what, expr)) {
        ::setenv(what[1].str().c_str(), what[2].str().c_str(), 1);
      }
    }
  }
}

} // namespace utils