#pragma once



#include <cereal/archives/json.hpp>


#include <sstream>
#include <iostream>

#include "generic_encoder.h"

#include "grlx/rpc/types.h"
#include "grlx/rpc/message.h"


namespace grlx
{
namespace rpc
{

using JsonEncoder = GenericEncoder<cereal::JSONOutputArchive, cereal::JSONInputArchive>;


}
}
