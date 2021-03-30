#include <grlx/net/http/authorization.hpp>

#include <boost/beast/core/detail/base64.hpp>

#include <grlx/net/alias/beast.hpp>

#include <cstdint>
#include <map>
#include <ostream>
#include <random>
#include <string>

namespace grlx::net::http::authorization {

namespace hex {
template <typename Ch>
inline std::string encode(Ch* data, size_t size) {
  std::string result;
  boost::algorithm::hex_lower(data, data + size, std::back_inserter(result));
  return result;
}
} // namespace hex

bearer::bearer(std::string_view token)
    : token_(token) {
}

std::string const& bearer::token() const noexcept {
  return token_;
}

std::ostream& operator<<(std::ostream& os, bearer const& bearer) {
  os << "Bearer " << bearer.token();
  return os;
}

basic::basic(std::string_view user, std::string_view password)
    : user_(user)
    , password_(password) {
}

std::string const& basic::user() const noexcept {
  return user_;
}

std::string const& basic::password() const noexcept {
  return password_;
}

std::ostream& operator<<(std::ostream& os, basic const& b) {
  using namespace beast::detail::base64;
  auto const source = b.user() + ':' + b.password();
  auto       out    = std::string(encoded_size(source.length()), '\0');
  out.resize(encode(out.data(), source.data(), source.length()));
  os << "Basic " << out;
  return os;
}

digest::digest(std::string_view user, std::string_view password, std::string_view authenticate)
    : user_(user)
    , password_(password)
    , authenticate_(authenticate)
    , m_qop{None} {
  find_nonce();
  find_realm();  
}

std::string const& digest::user() const noexcept {
  return user_;
}

std::string const& digest::password() const noexcept {
  return password_;
}

std::string const& digest::authorization() const noexcept {
  return m_authorization;
}
bool digest::generate_authorization(std::string_view method, std::string_view uri, std::string_view response_body) {
  find_opaque();
  find_qop(); 
  find_algorithm();
  // if (!ret)
  //   return false;

  m_uri        = uri;
  m_method     = method;
  m_body       = response_body;
  m_cnonce     = generate_nonce();
  m_nonceCount = update_nonce_count();
  MD5_hash ha1;
  calculate_ha1(ha1);
  m_ha1 = hex::encode(ha1, sizeof(ha1));
  MD5_hash ha2;
  calculate_ha2(ha2);
  m_ha2 = hex::encode(ha2, sizeof(ha2));
  MD5_hash response;
  calculate_response(response);
  m_response = hex::encode(response, sizeof(response));
  // std::ostringstream auth_ostr;
  // auth_ostr << "Digest username=\""
  //           << user_
  //           << "\", realm=\""
  //           << m_realm
  //           << "\", nonce=\""
  //           << m_nonce
  //           << "\", uri=\""
  //           << uri
  //           << "\", qop="
  //           << ((m_qop == AuthInt) ? "auth-int" : "auth")
  //           << ", algorithm=MD5, nc="
  //           << m_nonceCount
  //           << ", cnonce=\""
  //           << m_cnonce
  //           << "\", response=\""
  //           << m_response;
  // if (!m_opaque.empty()) {
  //   auth_ostr << "\", opaque=\""
  //             << m_opaque;
  // }
  // auth_ostr << "\"";

  m_authorization = "Digest username=\"";
  m_authorization.reserve(128 + user_.size() + m_realm.size() + m_nonce.size() +
                          m_uri.size() + m_nonceCount.size() + m_cnonce.size() +
                          m_response.size() + m_opaque.size());
  m_authorization.append(user_);
  m_authorization.append("\", realm=\"");
  m_authorization.append(m_realm);
  m_authorization.append("\", nonce=\"");
  m_authorization.append(m_nonce);
  m_authorization.append("\", uri=\"");
  m_authorization.append(m_uri);
  m_authorization.append("\", qop=");
  m_authorization.append(m_qop == AuthInt ? "auth-int" : "auth");
  m_authorization.append(", algorithm=MD5, nc=");
  m_authorization.append(m_nonceCount);
  m_authorization.append(", cnonce=\"");
  m_authorization.append(m_cnonce);
  m_authorization.append("\", response=\"");
  m_authorization.append(m_response);
  if (!m_opaque.empty()) {
    m_authorization.append("\", opaque=\"");
    m_authorization.append(m_opaque);
  }
  m_authorization.append("\"");  
  return true;
}

std::string digest::generate_nonce() {
  std::random_device                            rd;
  std::uniform_int_distribution<unsigned short> length{8, 32};
  std::uniform_int_distribution<unsigned short> distNum{0, 15};

  std::string nonce;
  nonce.resize(length(rd));
  constexpr char hex[16]{'0', '1', '2', '3', '4', '5', '6', '7',
                         '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  for (char& val : nonce) {
    val = hex[distNum(rd)];
  }
  return nonce;
}

std::string digest::update_nonce_count() {
  static unsigned int s_nonceCount{};
  std::stringstream   ss;
  if (++s_nonceCount > 99999999) {
    s_nonceCount = 1;
  }
  ss << std::setfill('0') << std::setw(8) << s_nonceCount;
  return ss.str();
}
bool digest::find_opaque() {
  return find_section("opaque", m_opaque);
}
bool digest::find_nonce() {
  return find_section("nonce", m_nonce);
}
bool digest::find_realm() {
  return find_section("realm", m_realm);
}
bool digest::find_algorithm() {
  return find_section("algorithm", m_algorithm);
}
bool digest::find_qop() {
  std::string_view qop;
  if (find_section("qop", qop)) {
    // auth-int only with response body - working with tested implementations
    if (boost::iequals(qop, "auth-int") && !m_body.empty()) {
      m_qop = AuthInt;
    } else {
      m_qop = Auth;
    }
  }
  return false;
}
bool digest::find_section(std::string const& key, std::string_view& value) const {
  boost::regex reg{key + "=([^,]+)"};
  auto         start = authenticate_.cbegin();
  auto         end   = authenticate_.cend();

  boost::match_flag_type                            flags = boost::match_default;
  boost::match_results<std::string::const_iterator> matches;

  if (boost::regex_search(start, end, matches, reg, flags)) {
    size_t size = static_cast<size_t>(std::distance(matches[1].first, matches[1].second));
    start       = matches[1].first;
    end         = matches[1].second -1;
    // Trim quotes if they are there.
    if (*start == '"') {
      ++start;
      --size;
    }
    if (*end == '"') {
      --size;      
    }
    value = std::string_view(start,end);
    return true;
  }
  return false;
}
void digest::calculate_ha1(MD5_hash ha1) noexcept {
  MD5_CTX Md5Ctx;
  MD5_Init(&Md5Ctx);
  MD5_Update(&Md5Ctx, user_.data(), user_.size());
  MD5_Update(&Md5Ctx, ":", 1);
  MD5_Update(&Md5Ctx, m_realm.data(), m_realm.size());
  MD5_Update(&Md5Ctx, ":", 1);
  MD5_Update(&Md5Ctx, password_.data(), password_.size());
  MD5_Final(ha1, &Md5Ctx);
  if (boost::iequals(m_algorithm, "md5-sess")) {
    MD5_Init(&Md5Ctx);
    MD5_Update(&Md5Ctx, ha1, MD5_DIGEST_LENGTH);
    MD5_Update(&Md5Ctx, ":", 1);
    MD5_Update(&Md5Ctx, m_nonce.data(), m_nonce.size());
    MD5_Update(&Md5Ctx, ":", 1);
    MD5_Update(&Md5Ctx, m_cnonce.data(), m_cnonce.size());
    MD5_Final(ha1, &Md5Ctx);
  }
}
void digest::calculate_ha2(MD5_hash ha2) noexcept {
  MD5_CTX Md5Ctx;
  // calculate H(A2)
  MD5_Init(&Md5Ctx);
  MD5_Update(&Md5Ctx, m_method.data(), m_method.size());
  MD5_Update(&Md5Ctx, ":", 1);
  MD5_Update(&Md5Ctx, m_uri.data(), m_uri.size());
  if (m_qop == AuthInt) {
    MD5_Update(&Md5Ctx, ":", 1);
    MD5_Update(&Md5Ctx, m_body.data(), m_body.size());
  }
  MD5_Final(ha2, &Md5Ctx);
}
void digest::calculate_response(MD5_hash result) noexcept {
  MD5_CTX Md5Ctx;
  // calculate response
  MD5_Init(&Md5Ctx);
  MD5_Update(&Md5Ctx, m_ha1.data(), m_ha1.size());
  MD5_Update(&Md5Ctx, ":", 1);
  MD5_Update(&Md5Ctx, m_nonce.data(), m_nonce.size());
  MD5_Update(&Md5Ctx, ":", 1);
  if (m_qop != None) {
    MD5_Update(&Md5Ctx, m_nonceCount.data(), m_nonceCount.size());
    MD5_Update(&Md5Ctx, ":", 1);
    MD5_Update(&Md5Ctx, m_cnonce.data(), m_cnonce.size());
    MD5_Update(&Md5Ctx, ":", 1);
    if (m_qop == AuthInt) {
      MD5_Update(&Md5Ctx, "auth-int", 8);
    } else {
      MD5_Update(&Md5Ctx, "auth", 4);
    }
    MD5_Update(&Md5Ctx, ":", 1);
  }
  MD5_Update(&Md5Ctx, m_ha2.data(), m_ha2.size());
  MD5_Final(result, &Md5Ctx);
}

std::ostream& operator<<(std::ostream& os, digest const& d) {
  os << d.authorization();
  return os;
}

std::ostream& operator<<(std::ostream& os, methods const& m) {
  std::visit([&os](auto&& m) { os << m; }, m);
  return os;
}

} // namespace grlx::net::http::authorization