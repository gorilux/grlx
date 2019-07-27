#pragma once

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
