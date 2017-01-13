/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some utility helper functions for tokenization/normalization
/// \file normalizer_ngram.hpp
#ifndef _STRUS_ANALYZER_TOKENIZER_HELPERS_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_HELPERS_HPP_INCLUDED
#include "strus/base/utf8.hpp"
#include "private/internationalization.hpp"

/// \brief strus toplevel namespace
namespace strus
{

static inline const char* skipChar( const char* si)
{
	unsigned char charsize = utf8charlen( *si);
	if (!charsize)
	{
		throw strus::runtime_error(_TXT( "illegal UTF-8 character in input: %u"), (unsigned int)(unsigned char)*si);
	}
	else
	{
		return si+charsize;
	}
}

class CharTable
{
public:
	CharTable( const char* op);

	bool operator[]( char ch) const
	{
		return m_ar[ (unsigned char)ch];
	}

private:
	bool m_ar[128];
};


bool wordBoundaryDelimiter( char const* si, const char* se);
bool whiteSpaceDelimiter( char const* si, const char* se);

}//namespace
#endif

