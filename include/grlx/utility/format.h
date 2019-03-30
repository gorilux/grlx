#pragma once


#include <string>
#include <cstring>
#include <cstdio>
#include <memory>
#include <cstdarg>

namespace grlx {

//template<typename ...TArgs>
//std::string format(const std::string& fmt, TArgs... args)
//{
//    std::string result;
//    int size = std::snprintf( 0, 0, fmt.c_str(), args...);

//    result.resize(size);

//    std::snprintf(const_cast<char*>(result.data()), size, fmt.c_str(), args...);

//    return result;

//}

inline std::string format(const std::string fmt_str, ...)
{
    int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while(1) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        std::strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());
}

}
