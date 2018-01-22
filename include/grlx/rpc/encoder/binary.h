#pragma once


#include "generic_encoder.h"
#include <cereal/archives/binary.hpp>
#include "grlx/rpc/types.h"
#include "grlx/rpc/message.h"


namespace grlx
{
namespace rpc
{

using BinaryEncoder = GenericEncoder<cereal::BinaryOutputArchive, cereal::BinaryInputArchive>;

}
}
