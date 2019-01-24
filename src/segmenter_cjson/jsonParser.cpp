/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "jsonParser.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/internationalization.hpp"
#include <cstdlib>
#include <stdexcept>
#include <setjmp.h>

using namespace strus;

static bool isSpace( char ch)
{
	return ((unsigned char)ch <= 32);
}
static bool isAlpha( char ch)
{
	return ((ch|32) >= 'a' && (ch|32) <= 'z') || ch == '-' || ch == '_';
}
static bool isDigit( char ch)
{
	return ch >= '0' && ch <= '9';
}
static bool isAlphaNum( char ch)
{
	return isAlpha(ch) || isDigit(ch);
}
static bool isStringQuote( char ch)
{
	return ch == '\'' || ch == '\"';
}

static char const* skipString( char const* si)
{
	char eb = *si++;
	for (; *si && *si != eb; ++si)
	{
		if (*si == '\\')
		{
			++si;
			if (!*si) return NULL;
		}
	}
	return *si ? ++si : NULL;
}
static char const* skipFloat( char const* si)
{
	for (; *si && isDigit(*si); ++si){}
	if (*si == '.')
	{
		for (++si; *si && isDigit(*si); ++si){}
	}
	return *si ? ++si : NULL;
}
static char const* skipValue( char const* si)
{
	if (*si == '-' || *si == '+')
	{
		si = skipFloat( ++si);
		if (!si) return NULL;
		if ((*si|32) == 'e')
		{
			++si;
			si = skipFloat( ++si);
		}
		return si;
	}
	else if (isDigit(*si))
	{
		return skipFloat( ++si);
	}
	else if (isAlpha(*si))
	{
		for (++si; *si && isAlphaNum(*si); ++si){}
		return si;
	}
	else if (isStringQuote(*si))
	{
		return skipString( si);
	}
	else
	{
		return NULL;
	}
}
static char const* skipSpaces( char const* si)
{
	for (; *si && isSpace(*si); ++si){}
	return si;
}
static char const* nextToken( char const* si)
{
	si = skipSpaces(si); 
	if (si == NULL || !*si)
	{
		throw std::runtime_error(_TXT("unexpected end of document"));
	}
	return si;
}

static const char* skipEndOfDocument( const char* str)
{
	enum State {Start,SeekFirstDef,SeekNextDef,SeekKey,SeekAssign,SeekValue};
	int brkcnt = 0;
	State state = Start;
	char const* si = str;
	for (;;)
	{
		switch (state)
		{
			case Start:
			{
				si = skipSpaces(si); 
				if (si == NULL)
				{
					if (brkcnt == 0) return NULL;
					throw std::runtime_error(_TXT("unexpected end of document"));
				}
				else if (!*si)
				{
					return NULL;
				}
				else if (*si == '{')
				{
					++si;
					++brkcnt;
					state = SeekFirstDef;
				}
				else
				{
					throw std::runtime_error(_TXT("expected open bracket '{'"));
				}
				break;
			}
			case SeekFirstDef:
			{
				si = nextToken( si); 
				if (*si == '}')
				{
					si = skipSpaces( ++si);
					if (brkcnt == 0) return si;
					--brkcnt;
					state = SeekNextDef;
				}
				else
				{
					state = SeekKey;
				}
				break;
			}
			case SeekNextDef:
			{
				si = nextToken(si); 
				if (*si == ',')
				{
					++si;
					state = SeekKey;
				}
				else if (*si == '}')
				{
					si=skipSpaces( ++si);
					if (brkcnt == 0) throw std::runtime_error(_TXT("syntax error, unexpected end of structure '}'"));
					if (--brkcnt == 0) return si;
					state = SeekNextDef;
				}
				else
				{
					throw std::runtime_error(_TXT("expected separator ',' or end of structure '}'"));
				}
				break;
			}
			case SeekKey:
			{
				si = nextToken(si); 
				if (isStringQuote(*si) || isAlpha(*si))
				{
					si = skipValue( si);
					if (si == NULL || !*si)
					{
						throw std::runtime_error(_TXT("unexpected end of document"));
					}
					state = SeekAssign;
				}
				else
				{
					throw std::runtime_error(_TXT("expected key"));
				}
				break;
			}
			case SeekAssign:
			{
				si = nextToken( si);
				if (*si == ':')
				{
					++si;
					state = SeekValue;
				}
				else
				{
					throw std::runtime_error(_TXT("expected assignment operator ':'"));
				}
				break;
			}
			case SeekValue:
			{
				si = nextToken(si); 
				if (*si == '{')
				{
					++si;
					++brkcnt;
					state = SeekFirstDef;
				}
				else if (isStringQuote(*si) || isAlpha(*si))
				{
					si = skipValue( si);
					if (si == NULL || !*si)
					{
						throw std::runtime_error(_TXT("unexpected end of document"));
					}
					state = SeekNextDef;
				}
				else
				{
					throw std::runtime_error(_TXT("expected value"));
				}
				break;
			}
		}
	}
	while ((unsigned char)*si <= 32) ++si;
}

const char* strus::jsonSkipEndOfNextDocument( const char* str)
{
	return skipEndOfDocument( str);
}


