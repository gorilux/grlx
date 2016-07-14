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

#ifndef GRLX_UTILITY_CONVERT_H
#define GRLX_UTILITY_CONVERT_H

#include "../details/config.h"
#include <string>
#include <iterator>
#include "convert_policy.h"

namespace grlx {
	/* --------------------------------------------------------------------- */
	//  convert
	/* --------------------------------------------------------------------- */
	template <class InIter, class OutIter, class ConvPolicy>
    inline OutIter convert(InIter first, InIter last, OutIter result, ConvPolicy f) {
		result = f.reset(result);
		for (; first != last; ++first) result = f(*first, result);
		result = f.finish(result);
		return result;
	}
	
	/* --------------------------------------------------------------------- */
	//  convert
	/* --------------------------------------------------------------------- */
	template <class Ch, class Tr, class ConvPolicy>
    inline std::basic_string<Ch, Tr> convert(const std::basic_string<Ch, Tr>& src, ConvPolicy f) {
		std::basic_string<Ch, Tr> dest;
		std::insert_iterator<std::basic_string<Ch, Tr> > out(dest, dest.end());
		grlx::convert(src.begin(), src.end(), out, f);
		return dest;
	}
	
	/* --------------------------------------------------------------------- */
	//  convert
	/* --------------------------------------------------------------------- */
	template <class CharT, class ConvPolicy>
	inline std::basic_string<CharT> convert(const CharT* src, ConvPolicy f) {
		std::basic_string<CharT> tmp(src);
		std::basic_string<CharT> dest;
		std::insert_iterator<std::basic_string<CharT> > out(dest, dest.end());
		grlx::convert(tmp.begin(), tmp.end(), out, f);
		return dest;
	}
} // namespace clx

#endif // CLX_CONVERT_H
