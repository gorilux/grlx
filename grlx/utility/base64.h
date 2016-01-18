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

#ifndef CLX_BASE64_H
#define CLX_BASE64_H

#include "../details/config.h"
#include <iterator>
#include <cstring>
#include <string>
#include <stdexcept>
#include "convert.h"
#include "base64_convert.h"

namespace grlx {
	namespace base64 {
		/* ----------------------------------------------------------------- */
		//  encode
		/* ----------------------------------------------------------------- */
		template <class InIter, class OutIter>
		inline OutIter encode(InIter first, InIter last, OutIter result) {
#if defined(__BORLANDC__)
			typedef char value_type;
#else
			typedef typename InIter::value_type value_type;
#endif
            grlx::basic_base64_encoder<value_type> f;
            return grlx::convert(first, last, result, f);
		}
		
		/* ----------------------------------------------------------------- */
		//  encode
		/* ----------------------------------------------------------------- */
		inline std::basic_string<char> encode(const std::basic_string<char>& src) {
			if (src.empty()) return std::basic_string<char>();
			std::basic_string<char> dest;
			std::insert_iterator<std::basic_string<char> > out(dest, dest.begin());
            grlx::base64::encode(src.begin(), src.end(), out);
			return dest;
		}
		
		/* ----------------------------------------------------------------- */
		//  encode
		/* ----------------------------------------------------------------- */
		inline std::basic_string<char> encode(const char* src) {
			std::basic_string<char> tmp(src);
            return grlx::base64::encode(tmp);
		}
		
		/* ----------------------------------------------------------------- */
		/*
		 *  encode
		 *
		 *  The function is deprecated. Use encode(InIter, InIter, OutIter)
		 */
		/* ----------------------------------------------------------------- */
		inline std::basic_string<char> encode(const char* src, std::size_t n) {
			if (n == 0) return std::basic_string<char>();
			std::basic_string<char> tmp(src, n);
            return grlx::base64::encode(tmp);
		}
		
		/* ----------------------------------------------------------------- */
		//  decode
		/* ----------------------------------------------------------------- */
		template <class InIter, class OutIter>
		inline OutIter decode(InIter first, InIter last, OutIter result) {
#if defined(__BORLANDC__)
			typedef char value_type;
#else
			typedef typename InIter::value_type value_type;
#endif
            grlx::basic_base64_decoder<value_type> f;
            return grlx::convert(first, last, result, f);
		}
		
		/* ----------------------------------------------------------------- */
		//  decode
		/* ----------------------------------------------------------------- */
		inline std::basic_string<char> decode(const std::basic_string<char>& src) {
			if (src.empty()) return std::basic_string<char>();
			std::basic_string<char> dest;
			std::insert_iterator<std::basic_string<char> > out(dest, dest.begin());
            grlx::base64::decode(src.begin(), src.end(), out);
			return dest;
		}
		
		/* ----------------------------------------------------------------- */
		//  decode
		/* ----------------------------------------------------------------- */
		inline std::basic_string<char> decode(const char* src) {
			std::basic_string<char> tmp(src);
            return grlx::base64::decode(tmp);
		}
		
		/* ----------------------------------------------------------------- */
		/*
		 *  decode
		 *
		 *  The function is deprecated. Use decode(InIter, InIter, OutIter)
		 *  or decode(const char*).
		 */
		/* ----------------------------------------------------------------- */
		inline std::basic_string<char> decode(const char* src, size_t n) {
			if (n == 0) return std::basic_string<char>();
			std::basic_string<char> tmp(src, n);
            return grlx::base64::decode(tmp);
		}
	}
}

#endif // CLX_BASE64_H
