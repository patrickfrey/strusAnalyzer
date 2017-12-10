/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some utility helper functions for tokenization/normalization
/// \file tokenizeHelpers.cpp
#include "private/tokenizeHelpers.hpp"
#include "private/unicodeWordDelimiters.hpp"
#include "strus/base/utf8.hpp"


/// \brief strus toplevel namespace
using namespace strus;

CharTable::CharTable( const char* op)
{
	std::size_t ii;
	for (ii=0; ii<sizeof(m_ar); ++ii) m_ar[ii] = false;
	for (ii=0; op[ii]; ++ii)
	{
		m_ar[(unsigned char)(op[ii])] = true;
	}
}

bool strus::wordBoundaryDelimiter( char const* si, const char* se)
{
	static const CharTable wordCharacter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
	static const UnicodeWordDelimiters unicodeWordDelimiters;
	if ((unsigned char)*si <= 32)
	{
		return true;
	}
	else if ((unsigned char)*si >= 128)
	{
		unsigned int chr = utf8decode( si, utf8charlen(*si));
		if (unicodeWordDelimiters.find( chr) != unicodeWordDelimiters.end()) return true;
		return false;
	}
	else if (wordCharacter[ *si])
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool strus::whiteSpaceDelimiter( char const* si, const char* se)
{
	if ((unsigned char)*si <= 32)
	{
		return true;
	}
	else if ((unsigned char)*si >= 128)
	{
		unsigned int chr = utf8decode( si, utf8charlen(*si));
		if (chr == 133) return true;
		if (chr >= 0x2000 && chr <= 0x200F) return true;
		if (chr >= 0x2028 && chr <= 0x2029) return true;
		if (chr == 0x202F) return true;
		if (chr >= 0x205F && chr <= 0x2060) return true;
		if (chr == 0x3000) return true;
		if (chr == 0xFEFF) return true;
		return false;
	}
	else
	{
		return false;
	}
}

