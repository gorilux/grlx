#pragma once

#include <array>
#include <string>
#include <string_view>

#include <iosfwd>

namespace grlx::net::http {

class Content_Type : private std::array<std::string, 3> {
  using base_t = std::array<std::string, 3>;

public:
  Content_Type() = default;

  explicit Content_Type(std::string_view type,
                        std::string_view charset  = "",
                        std::string_view boundary = "");

private:
  explicit Content_Type(std::string&& type,
                        std::string&& charset,
                        std::string&& boundary);

public:
  static Content_Type parse(std::string_view sv);

  std::string const& type() const;
  void               type(std::string_view);
  std::string const& charset() const;
  void               charset(std::string_view);
  std::string const& boundary() const;
  void               boundary(std::string_view);
};

bool operator==(Content_Type const& lhs, Content_Type const& rhs);
bool operator!=(Content_Type const& lhs, Content_Type const& rhs);

std::ostream& operator<<(std::ostream& os, Content_Type const& ct);

} // namespace grlx::net::http