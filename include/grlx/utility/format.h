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
#ifndef GRLX_UTILITY_FORMAT_H
#define GRLX_UTILITY_FORMAT_H

#include <string>
#include <cstdio>

namespace grlx {

template<typename ...TArgs>
std::string format(const std::string& fmt, TArgs... args)
{
    std::string result;
    int size = std::snprintf( 0, 0, fmt.c_str(), args...);

    result.resize(size);

    std::snprintf(const_cast<char*>(result.data()), size, fmt.c_str(), args...);

    return result;

}

}

#endif // GRLX_UTILITY_FORMAT_H
