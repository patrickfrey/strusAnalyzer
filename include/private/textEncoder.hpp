/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Conversion of character set encodings to UTF-8
#ifndef _STRUS_UTILS_TEXT_ENCODER_HPP_INCLUDED
#define _STRUS_UTILS_TEXT_ENCODER_HPP_INCLUDED
#include <string>

namespace strus {
namespace utils {

class TextEncoderBase
{
public:
	virtual ~TextEncoderBase(){}
	virtual std::string convert( const char* src, std::size_t srcsize, bool eof)=0;
};

TextEncoderBase* createTextEncoder( const char* charset);

const char* detectBOM( const char* src, std::size_t srcsize, std::size_t& BOM_size);
const char* detectCharsetEncoding( const char* src, std::size_t srcsize);

}}//namespace
#endif

