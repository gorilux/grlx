#pragma once

#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/convert.hpp>
#include <boost/convert/strtol.hpp>
#include <boost/regex.hpp>

#include <openssl/md5.h>

#include <iosfwd>
#include <string>
#include <string_view>
#include <variant>
//#include <boost/variant2/variant.hpp>

namespace grlx::net::http::authorization {

class bearer {
  std::string token_;

public:
  bearer() = delete;
  bearer(std::string_view token);
  std::string const& token() const noexcept;
};

std::ostream& operator<<(std::ostream& os, bearer const& ct);

class basic {
  std::string user_;
  std::string password_;

public:
  basic() = delete;
  basic(std::string_view user, std::string_view password);

  std::string const& user() const noexcept;
  std::string const& password() const noexcept;
};

std::ostream& operator<<(std::ostream& os, basic const& ct);

class digest {

  enum QualityOfProtection {
    None,
    Auth,
    AuthInt
  };
  std::string         user_;
  std::string         password_;
  std::string         authenticate_;
  std::string         m_authorization;
  std::string         m_cnonce;
  std::string         m_nonceCount;
  std::string         m_ha1;
  std::string         m_ha2;
  std::string         m_response;
  QualityOfProtection m_qop;
  std::string_view    m_realm;
  std::string_view    m_nonce;
  std::string_view    m_opaque;
  std::string_view    m_algorithm;
  std::string_view    m_uri;
  std::string_view    m_method;
  std::string_view    m_body;

public:
  digest() = delete;
  digest(std::string_view user, std::string_view password, std::string_view authenticate);
  std::string const& user() const noexcept;
  std::string const& password() const noexcept;
  std::string const& authorization() const noexcept;
  bool               generate_authorization(std::string_view target, std::string_view method, std::string_view response_body);

private:
  using MD5_hash = unsigned char[MD5_DIGEST_LENGTH];
  std::string generate_nonce();
  std::string update_nonce_count();
  bool        find_opaque();
  bool        find_nonce();
  bool        find_realm();
  bool        find_algorithm();
  bool        find_qop();
  bool        find_section(std::string const& key, std::string_view& value) const;
  void        calculate_ha1(MD5_hash ha1) noexcept;
  void        calculate_ha2(MD5_hash ha2) noexcept;
  void        calculate_response(MD5_hash result) noexcept;
};

std::ostream& operator<<(std::ostream& os, digest const& ct);

//using methods = boost::variant2::variant<basic, bearer, digest>;
using methods = std::variant<basic, bearer, digest>;

std::ostream& operator<<(std::ostream& os, methods const& m);

} // namespace grlx::net::http::authorization