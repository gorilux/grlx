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
#ifndef RPC_RPCTYPES_H
#define RPC_RPCTYPES_H

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


namespace Details{


struct InvalidType{};
struct RequestType{};
struct ReplyType{};
struct NotificationType{};
struct ErrorType{};
struct FundamentalType{};
struct ComplexType{};
struct TupleType{};
struct StdContainerType{};

}

const int32_t kStdLibTypes = 50;
const int32_t kQtLibTypes  = 60;
const int32_t kCustomTypes = 100;



template<typename ...ArgsT>
struct Request;

template<typename ResultT>
struct Reply;

template<typename ...ArgsT>
struct Notification;

struct Error;


template<typename T>
struct TypeID;


template<int V>
using IntConst = std::integral_constant<int, V>;

const int32_t kDataTypeUnknown = -1;


template<typename T>
struct TypeID: IntConst<kDataTypeUnknown>
{
    typedef Details::ComplexType Tag;
};

template<>
struct TypeID<void> : IntConst<0>
{
    typedef Details::FundamentalType Tag;
};

template<>
struct TypeID<char> : IntConst<1>
{
    typedef Details::FundamentalType Tag;
};

template<>
struct TypeID<int8_t> : IntConst<1>
{
    typedef Details::FundamentalType Tag;
};
template<>
struct TypeID<uint8_t> : IntConst<2>
{
    typedef Details::FundamentalType Tag;
};
template<>
struct TypeID<int16_t> : IntConst<3>
{
    typedef Details::FundamentalType Tag;
};
template<>
struct TypeID<uint16_t> : IntConst<4>
{
    typedef Details::FundamentalType Tag;
};
template<>
struct TypeID<int32_t> : IntConst<5>
{
    typedef Details::FundamentalType Tag;
};
template<>
struct TypeID<uint32_t> : IntConst<6>
{
    typedef Details::FundamentalType Tag;
};
template<>
struct TypeID<int64_t>  : IntConst<7>
{
    typedef Details::FundamentalType Tag;
};
template<>
struct TypeID<uint64_t> : IntConst<8>
{
    typedef Details::FundamentalType Tag;
};
template<>
struct TypeID<float> : IntConst<9>
{
    typedef Details::FundamentalType Tag;
};
template<>
struct TypeID<double> : IntConst<10>
{
    typedef Details::FundamentalType Tag;
};
template<>
struct TypeID<bool> : IntConst<11>
{
    typedef Details::FundamentalType Tag;
};


template<typename T, int SZ>
struct TypeID<T[SZ]> : IntConst<12>
{
    typedef Details::FundamentalType Tag;
};


template<>
struct TypeID<const char*> : IntConst<13>
{
    typedef Details::FundamentalType Tag;
};

template<typename ...ArgsT>
struct TypeID<std::tuple<ArgsT...>> : IntConst<20>
{
    typedef Details::TupleType Tag;
};

template<typename ...ArgsT>
struct TypeID<Request<ArgsT...>> : IntConst<21>
{
    typedef Details::RequestType Tag;
};

template<typename ResultT>
struct TypeID<Reply<ResultT>> : IntConst<22>
{
    typedef Details::ReplyType Tag;
};

template<typename ...ArgsT>
struct TypeID<Notification<ArgsT...>> : IntConst<23>
{
    typedef Details::NotificationType Tag;
};

template<>
struct TypeID<Error> : IntConst<24>
{
    typedef Details::ErrorType Tag;
};

template<>
struct TypeID<std::string>   : IntConst<kStdLibTypes + 0>
{
    typedef Details::StdContainerType Tag;
};

template<typename T>
struct TypeID<std::vector<T>> : IntConst<kStdLibTypes + 1>
{
    typedef Details::StdContainerType Tag;
};

template<typename T>
struct TypeID<std::list<T>> : IntConst<kStdLibTypes + 2>
{
    typedef Details::StdContainerType Tag;
};

template<typename T>
struct TypeID<std::deque<T>> : IntConst<kStdLibTypes + 3>
{
    typedef Details::StdContainerType Tag;
};

template<typename T>
struct TypeID<std::set<T>> : IntConst<kStdLibTypes + 4>
{
    typedef Details::StdContainerType Tag;
};
template<typename T>
struct TypeID<std::multiset<T>> : IntConst<kStdLibTypes + 5>
{
    typedef Details::StdContainerType Tag;
};

template<typename T>
struct TypeID<std::unordered_set<T>> : IntConst<kStdLibTypes + 6>
{
    typedef Details::StdContainerType Tag;
};

template<typename T>
struct TypeID<std::unordered_multiset<T>> : IntConst<kStdLibTypes + 7>
{
    typedef Details::StdContainerType Tag;
};

template<typename K, typename U>
struct TypeID<std::map<K,U>> : IntConst<kStdLibTypes + 8>
{
    typedef Details::StdContainerType Tag;
};
template<typename K, typename U>
struct TypeID<std::multimap<K,U>> : IntConst<kStdLibTypes + 9>
{
    typedef Details::StdContainerType Tag;
};
template<typename K, typename U>
struct TypeID<std::unordered_map<K,U>> : IntConst<kStdLibTypes + 10>
{
    typedef Details::StdContainerType Tag;
};
template<typename K, typename U>
struct TypeID<std::unordered_multimap<K,U>> : IntConst<kStdLibTypes + 11>
{
    typedef Details::StdContainerType Tag;
};


namespace Details
{
    class DummyBaseClass{ public: };
    class DummyBaseStruct { public: };
}


}

}
#endif // RPC_RPCTYPES_H
