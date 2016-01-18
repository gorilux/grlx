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

#ifndef CLX_BASE64_CONVERT_H
#define CLX_BASE64_CONVERT_H

#include "../details/config.h"
#include <cassert>
#include <string>
#include <stdexcept>
#include "literal.h"

namespace grlx {
	/* --------------------------------------------------------------------- */
	//  basic_base64_encoder
	/* --------------------------------------------------------------------- */
	template <
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	class basic_base64_encoder {
	public:
		typedef std::size_t size_type;
		typedef CharT char_type;
		typedef std::basic_string<CharT, Traits> string_type;
		
		basic_base64_encoder() : buffer_() {}
		
		~basic_base64_encoder() throw() {}
		
		template <class OutIter>
		OutIter operator()(char_type c, OutIter out) {
			assert(buffer_.size() < 3);
			
			buffer_ += c;
			if (buffer_.size() == 3) {
				*out++ = table().at((buffer_.at(0) & 0xfc) >> 2);
				*out++ = table().at(((buffer_.at(0) & 0x03) << 4) | ((buffer_.at(1) & 0xf0) >> 4));
				*out++ = table().at(((buffer_.at(1) & 0x0f) << 2) | ((buffer_.at(2) & 0xc0) >> 6));
				*out++ = table().at((buffer_.at(2) & 0x3f));
				buffer_.clear();
			}
			
			return out;
		}
		
		template <class OutIter>
		OutIter reset(OutIter out) {
			buffer_.clear();
			return out;
		}
		
		template <class OutIter>
		OutIter finish(OutIter out) {
			assert(buffer_.size() < 3);
			
			switch (buffer_.size()) {
			case 1:
				buffer_ += static_cast<char_type>(0);
				buffer_ += static_cast<char_type>(0);
				*out++ = table().at((buffer_.at(0) & 0xfc) >> 2);
				*out++ = table().at(((buffer_.at(0) & 0x03) << 4) | ((buffer_.at(1) & 0xf0) >> 4));
				*out++ = table().at(64);
				*out++ = table().at(64);
				break;
			case 2:
				buffer_ += static_cast<char_type>(0);
				*out++ = table().at((buffer_.at(0) & 0xfc) >> 2);
				*out++ = table().at(((buffer_.at(0) & 0x03) << 4) | ((buffer_.at(1) & 0xf0) >> 4));
				*out++ = table().at(((buffer_.at(1) & 0x0f) << 2) | ((buffer_.at(2) & 0xc0) >> 6));
				*out++ = table().at(64);
				break;
			default:
				break;
			}
			
			buffer_.clear();
			return out;
		}
		
	private:
		string_type buffer_;
		
		static const string_type& table() {
			static const string_type s(LITERAL("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="));
			return s;
		}
	};
	
	/* --------------------------------------------------------------------- */
	//  basic_base64_decoder
	/* --------------------------------------------------------------------- */
	template <
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	class basic_base64_decoder {
	public:
		typedef std::size_t size_type;
		typedef CharT char_type;
		typedef std::basic_string<CharT, Traits> string_type;
		
		basic_base64_decoder() : buffer_() {}
		
		~basic_base64_decoder() throw() {}
		
		template <class OutIter>
		OutIter operator()(char_type c, OutIter out) {
			assert(buffer_.size() < 4);
			
			if (c != table().at(64)) {
				size_type pos = table().find(c);
				if (pos == string_type::npos) throw std::runtime_error("wrong base64 encoded data");
				buffer_ += static_cast<char_type>(pos);
			}
			
			if (buffer_.size() == 4) {
				*out++ = ((buffer_.at(0) << 2) & 0xfc) | ((buffer_.at(1) >> 4) & 0x03);
				*out++ = ((buffer_.at(1) << 4) & 0xf0) | ((buffer_.at(2) >> 2) & 0x0f);
				*out++ = ((buffer_.at(2) << 6) & 0xc0) | (buffer_.at(3) & 0x3f);
				buffer_.clear();
			}
			
			return out;
		}
		
		template <class OutIter>
		OutIter reset(OutIter out) {
			buffer_.clear();
			return out;
		}
		
		template <class OutIter>
		OutIter finish(OutIter out) {
			if (!buffer_.empty()) {
				assert(buffer_.size() >= 2 && buffer_.size() <= 4);
				
				string_type tmp(buffer_);
				buffer_.clear();
				
				*out++ = ((tmp.at(0) << 2) & 0xfc) | ((tmp.at(1) >> 4) & 0x03);
				if (tmp.size() == 2) return out;
				*out++ = ((tmp.at(1) << 4) & 0xf0) | ((tmp.at(2) >> 2) & 0x0f);
				if (tmp.size() == 3) return out;
				*out++ = ((tmp.at(2) << 6) & 0xc0) | (tmp.at(3) & 0x3f);
				
			}
			
			return out;
		}
		
	private:
		string_type buffer_;
		
		static const string_type& table() {
			static const string_type s(LITERAL("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="));
			return s;
		}
	};
	
	typedef basic_base64_encoder<char, std::char_traits<char> > base64_encoder;
	typedef basic_base64_decoder<char, std::char_traits<char> > base64_decoder;
} // namespace clx

#endif // CLX_BASE64_CONVERT_H
