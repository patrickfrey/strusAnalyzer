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

bool strus::queryFieldDelimiter( char const* si, const char* se)
{
	return *si == '"' || *si == ';';
}

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
// --------------------------------------------------------------------------------------
// Table used for identifying white-spaces
// List edited, original from Jukka K. Korpela, "IT and communication" Yucca's free information site
//	Link http://jkorpela.fi/chars/spaces.html
// --------------------------------------------------------------------------------------
// U+0020	SPACE	foo bar	Depends on font, typically 1/4 em, often adjusted
// U+00A0	NO-BREAK SPACE	foo bar	As a space, but often not adjusted
// U+1680	OGHAM SPACE MARK	foo bar	Unspecified; usually not really a space but a dash
// U+180E	MONGOLIAN VOWEL SEPARATOR	foo᠎bar	0
// U+2000	EN QUAD	foo bar	1 en (= 1/2 em)
// U+2001	EM QUAD	foo bar	1 em (nominally, the height of the font)
// U+2002	EN SPACE (nut)	foo bar	1 en (= 1/2 em)
// U+2003	EM SPACE (mutton)	foo bar	1 em
// U+2004	THREE-PER-EM SPACE (thick space)	foo bar	1/3 em
// U+2005	FOUR-PER-EM SPACE (mid space)	foo bar	1/4 em
// U+2006	SIX-PER-EM SPACE	foo bar	1/6 em
// U+2007	FIGURE SPACE	foo bar	“Tabular width”, the width of digits
// U+2008	PUNCTUATION SPACE	foo bar	The width of a period “.”
// U+2009	THIN SPACE	foo bar	1/5 em (or sometimes 1/6 em)
// U+200A	HAIR SPACE	foo bar	Narrower than THIN SPACE
// U+200B	ZERO WIDTH SPACE	foo​bar	0
// U+200C	ZERO WIDTH NON-JOINER
// U+200D	ZERO WIDTH JOINER
// U+2028	LINE SEPARATOR	
// U+2029	PARAGRAPH SEPARATOR	
// U+202A	LEFT-TO-RIGHT EMBEDDING
// U+202B	RIGHT-TO-LEFT EMBEDDING
// U+202C	POP DIRECTIONAL FORMATTING
// U+202D	LEFT-TO-RIGHT OVERRIDE
// U+202E	RIGHT-TO-LEFT OVERRIDE
// U+202F	NARROW NO-BREAK SPACE	foo bar	Narrower than NO-BREAK SPACE (or SPACE), “typically the width of a thin space or a mid space”
// U+205F	MEDIUM MATHEMATICAL SPACE	foo bar	4/18 em
// U+3000	IDEOGRAPHIC SPACE	foo　bar	The width of ideographic (CJK) characters.
// U+FEFF	ZERO WIDTH NO-BREAK SPACE
	if ((unsigned char)*si <= 32)
	{
		return true;
	}
	else if ((unsigned char)*si >= 128)
	{
		unsigned int chr = utf8decode( si, utf8charlen(*si));
		if (chr == 133/*NEL Next line*/) return true;
		if (chr == 0xA0/*NO-BREAK SPACE*/) return true;
		if (chr >= 0x1680)
		{
			if (chr <= 0x2060)
			{
				if (chr < 0x2000)
				{
					if (chr == 0x1680) return true;
					if (chr == 0x180E) return true;
				}
				else
				{
					if (chr >= 0x2000 && chr <= 0x200D) return true;
					if (chr >= 0x2028 && chr <= 0x202F) return true;
					if (chr >= 0x205F && chr <= 0x2060) return true;
				}
			}
			else
			{
				if (chr == 0x3000) return true;
				if (chr == 0xFEFF) return true;
			}
		}
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
	for (; si < se && strus::whiteSpaceDelimiter( si, se); si += strus::utf8charlen(*si)){}
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


