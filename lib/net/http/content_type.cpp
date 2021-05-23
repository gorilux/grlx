#include <grlx/net/http/content_type.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <regex>

namespace grlx::net::http {
Content_Type::Content_Type(std::string_view type,
                           std::string_view charset,
                           std::string_view boundary)
    : base_t{std::string(type), std::string(charset), std::string(boundary)} {
}

Content_Type::Content_Type(std::string&& type,
                           std::string&& charset,
                           std::string&& boundary)
    : base_t{std::move(type), std::move(charset), std::move(boundary)} {
}

Content_Type Content_Type::parse(std::string_view sv) {
  // https://regex101.com/r/JGZNn8/2/
  static auto const reg = std::regex(
      R"(^([\w!#$&\-^]{1,127}?\/[\w!#$&\-^]{1,127}?)(?:;\s*charset=([\w!#$%^&\-~{}]{1,127}?))?(?:;\s*boundary=([\w!#$%^&\-~{}]{1,70}?))?\s*$)",
      std::regex_constants::icase | std::regex_constants::optimize);
  std::smatch match;
  auto const  str = std::string{sv};
  if (!std::regex_match(str.cbegin(), str.cend(), match, reg))
    throw std::domain_error("malformed content type");
  return Content_Type(match[1], match[2], match[3]);
}

std::string const& Content_Type::type() const {
  return std::get<0>(*this);
}

void Content_Type::type(std::string_view t) {
  std::get<0>(*this) = t;
}

std::string const& Content_Type::charset() const {
  return std::get<1>(*this);
}

void Content_Type::charset(std::string_view c) {
  std::get<1>(*this) = c;
}

std::string const& Content_Type::boundary() const {
  return std::get<2>(*this);
}

void Content_Type::boundary(std::string_view b) {
  std::get<2>(*this) = b;
}

bool operator==(Content_Type const& lhs, Content_Type const& rhs) {
  return boost::iequals(lhs.type(), rhs.type()) &&
         boost::iequals(lhs.charset(), rhs.charset()) &&
         lhs.boundary() == rhs.boundary();
}

bool operator!=(Content_Type const& lhs, Content_Type const& rhs) {
  return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, Content_Type const& ct) {
  os << ct.type();
  if (!ct.charset().empty())
    os << "; charset=" << ct.charset();
  if (!ct.boundary().empty())
    os << "; boundary=" << ct.boundary();
  return os;
}

} // namespace grlx::net::http