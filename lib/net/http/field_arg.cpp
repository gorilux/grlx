#include <grlx/net/http/field_arg.hpp>

namespace grlx::net::http {

field_arg::field_arg(http::field field, std::string value)
  : field_name(to_string(field)),
    value(std::move(value))
{
}

field_arg::field_arg(std::string name, std::string value)
  : 
    field_name(std::move(name)),
    value(std::move(value))
{
}


}