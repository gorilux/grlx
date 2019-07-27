#pragma once

#include <exception>
#include <stdexcept>

namespace grlx
{
namespace rpc
{

class TimeoutException : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

}
}
