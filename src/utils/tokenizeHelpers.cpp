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
static const CharTable g_wordCharacter("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
static const UnicodeWordDelimiters g_unicodeWordDelimiters;

bool strus::wordBoundaryDelimiter( char const* si, const char* se)
{
	if ((unsigned char)*si <= 32)
	{
		return true;
	}
	else if ((unsigned char)*si >= 128)
	{
		unsigned int chr = utf8decode( si, utf8charlen(*si));
		if (g_unicodeWordDelimiters.find( chr) != g_unicodeWordDelimiters.end()) return true;
		return false;
	}
	else if (g_wordCharacter[ *si])
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

SourceSpan strus::getNextPosTaggingEntity( char const* src, int len, int& pos)
{
	char const* si = src + pos;
	const char* se = src + len;
	for (; si < se && (unsigned char)*si <= 32; ++si){}
	if (si >= se) return SourceSpan();

	const char* start = si;
	while (si < se)
	{
		if ((unsigned char)*si >= 128)
		{
			int chrlen = utf8charlen(*si);
			unsigned int chr = utf8decode( si, chrlen);
			if (g_unicodeWordDelimiters.find( chr) != g_unicodeWordDelimiters.end())
			{
				if (si == start)
				{
					//... one delimiter character only, return the character
					SourceSpan rt( start - src, chrlen);
					pos = si - src + chrlen;
					return rt;
				}
				else
				{
					//... delimiter character and a non empty sequence of non space characters passed, return the sequence
					SourceSpan rt( start - src, si - start);
					pos = si - src;
					return rt;
				}
			}
			else
			{
				si += chrlen;
			}
		}
		else if (g_wordCharacter[ *si])
		{
			++si;
			continue;
		}
		else if ((unsigned char)*si <= 32)
		{
			// ... space, return the sequence passed till now
			SourceSpan rt( start - src, si - start);
			pos = si - src;
			return rt;
		}
		else
		{
			if (si == start)
			{
				//... one single byte delimiter character only, return the character
				SourceSpan rt( start - src, 1);
				pos = si - src + 1;
				return rt;
			}
			else
			{
				//... delimiter character and a non empty sequence of non space characters passed, return the sequence
				SourceSpan rt( start - src, si - start);
				pos = si - src;
				return rt;
			}
		}
	}
	SourceSpan rt( start - src, si - start);
	pos = si - src;
	return rt;
}


