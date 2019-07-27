#pragma once

////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2015 David Salvador Pinheiro
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is David Salvador Pinheiro
///
/// @author David Salvador Pinheiro
/// @author Copyright 2015, David Salvador Pinheiro
////////////////////////////////////////////////////////////////////////////////


#include <vector>
#include <cstdint>
#include <type_traits>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <any>


namespace grlx {

namespace rpc{


using ByteArray = std::vector<uint8_t>;

using MsgHandler         = std::function<int(const char*, int size)>;

struct MsgType
{
    enum Type
    {
        Invalid,
        Request,
        Response,
        Notification,
        Error
    };
};

struct ConnectionRole
{
   enum Type
   {
        Server,
        Client
   };
};




template<typename TReturn, typename ...ArgsT>
struct Request;

template<typename ResultT>
struct Reply;

template<typename ...ArgsT>
struct Notification;

struct Error;



namespace Details
{

class DummyBaseClass{ public: };
class DummyBaseStruct { public: };
class None { public: };


}

using TokenType = std::any;

}

}
