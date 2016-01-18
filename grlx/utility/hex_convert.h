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


#ifndef CLX_HEX_CONVERT_H
#define CLX_HEX_CONVERT_H

#include "../details/config.h"
#include <iomanip>
#include <iterator>
#include <string>
#include <sstream>
#include <grlx/tmpl/bitmask.h>
#include "literal.h"

namespace grlx {
	/* --------------------------------------------------------------------- */
	//  basic_hex_encoder
	/* --------------------------------------------------------------------- */
	template <
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	class basic_hex_encoder {
	public:
		typedef std::size_t size_type;
		typedef CharT char_type;
		typedef Traits traits;
		typedef std::basic_string<CharT, Traits> string_type;
		
		explicit basic_hex_encoder(bool lower = true) :
			prefix_(), lower_(lower) {}
		
		explicit basic_hex_encoder(const string_type& prefix, bool lower = true) :
			prefix_(prefix), lower_(lower) {}
		
		explicit basic_hex_encoder(const char_type* prefix, bool lower = true) :
			prefix_(prefix), lower_(lower) {}
		
		basic_hex_encoder(const string_type& prefix, const string_type& suffix, bool lower = true) :
			prefix_(prefix), suffix_(suffix), lower_(lower) {}
		
		basic_hex_encoder(const char_type* prefix, const char_type* suffix, bool lower = true) :
			prefix_(prefix), suffix_(suffix), lower_(lower) {}
		
		~basic_hex_encoder() throw() {}
		
		template <class OutIter>
		OutIter operator()(char_type c, OutIter out) {
			static const size_type width = (sizeof(char_type) > 1) ? 4 : 2;
			
			std::basic_stringstream<char_type, traits> ss;
			if (!prefix_.empty()) ss << prefix_;
			if (!lower_) ss << std::uppercase;
			ss << std::setw(width) << std::setfill((char_type)LITERAL('0')) << std::hex;
            ss << (static_cast<size_type>(c) & tmpl::lower_mask<sizeof(char_type) * 8>::value);
			if (!suffix_.empty()) ss << suffix_;
			
			std::istreambuf_iterator<char_type> first(ss);
			std::istreambuf_iterator<char_type> last;
			return std::copy(first, last, out);
		}
		
		template <class OutIter>
		OutIter reset(OutIter out) {
			return out;
		}
		
		template <class OutIter>
		OutIter finish(OutIter out) {
			return out;
		}
		
	private:
		string_type prefix_;
		string_type suffix_;
		bool lower_;
	};
	
	typedef basic_hex_encoder<char, std::char_traits<char> > hex_encoder;
#ifdef CLX_USE_WCHAR
	typedef basic_hex_encoder<wchar_t, std::char_traits<wchar_t> > whex_encoder;
#endif // CLX_USE_WCHAR
} // namespace clx

#endif // CLX_HEX_FILTER_H
