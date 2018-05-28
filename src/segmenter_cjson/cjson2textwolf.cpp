/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "cjson2textwolf.hpp"
#include "private/internationalization.hpp"
#include <utility>

using namespace strus;

static void getTextwolfValue( std::vector<TextwolfItem>& tiar, cJSON const* nd, const char* value)
{
	typedef textwolf::XMLScannerBase TX;
	if (nd->string)
	{
		if (nd->string[0] == '-')
		{
			tiar.push_back( TextwolfItem( TX::TagAttribName, nd->string+1));
			tiar.push_back( TextwolfItem( TX::TagAttribValue, value));
		}
		else if (nd->string[0] == '#' && std::strcmp( nd->string, "#text") == 0)
		{
			tiar.push_back( TextwolfItem( TX::Content, value));
		}
		else
		{
			tiar.push_back( TextwolfItem( TX::OpenTag, nd->string));
			tiar.push_back( TextwolfItem( TX::Content, value));
			tiar.push_back( TextwolfItem( TX::CloseTag, nd->string));
		}
	}
	else
	{
		tiar.push_back( TextwolfItem( TX::Content, value));
	}
}

void strus::getTextwolfItems( std::vector<TextwolfItem>& itemar, cJSON const* nd)
{
	typedef textwolf::XMLScannerBase TX;
	switch (nd->type & 0x7F)
	{
		case cJSON_False:
			getTextwolfValue( itemar, nd, "false");
			break;
		case cJSON_True:
			getTextwolfValue( itemar, nd, "true");
			break;
		case cJSON_NULL:
			if (nd->string && nd->string[0] != '-' && nd->string[0] != '#')
			{
				itemar.push_back( TextwolfItem( TX::OpenTag, nd->string));
				itemar.push_back( TextwolfItem( TX::CloseTagIm));
			}
			break;
		case cJSON_String:
			getTextwolfValue( itemar, nd, nd->valuestring);
			break;
		case cJSON_Number:
			if (!nd->valuestring)
			{
				throw strus::runtime_error( "%s",  _TXT("value node without string value found in JSON structure"));
			}
			getTextwolfValue( itemar, nd, nd->valuestring);
			break;
		case cJSON_Array:
		{
			cJSON const* chnd = nd->child;
			if (nd->string)
			{
				for (;chnd; chnd = chnd->next)
				{
					itemar.push_back( TextwolfItem( TX::OpenTag, nd->string));
					getTextwolfItems( itemar, chnd);
					itemar.push_back( TextwolfItem( TX::CloseTag, nd->string));
				}
			}
			else
			{
				unsigned int idx=0;
				char idxstr[ 64];
				for (;chnd; chnd = chnd->next)
				{
					snprintf( idxstr, sizeof( idxstr), "%u", idx);
					itemar.push_back( TextwolfItem( TX::OpenTag, idxstr));
					getTextwolfItems( itemar, chnd);
					itemar.push_back( TextwolfItem( TX::CloseTag, idxstr));
				}
			}
			break;
		}
		case cJSON_Object:
		{
			cJSON const* chnd = nd->child;
			if (nd->string)
			{
				itemar.push_back( TextwolfItem( TX::OpenTag, nd->string));
				for (;chnd; chnd = chnd->next)
				{
					getTextwolfItems( itemar, chnd);
				}
				itemar.push_back( TextwolfItem( TX::CloseTag, nd->string));
			}
			else
			{
				for (;chnd; chnd = chnd->next)
				{
					getTextwolfItems( itemar, chnd);
				}
			}
			break;
		}
		default:
			throw strus::runtime_error( _TXT("internal: memory corruption found in JSON structure"));
	}
}

static std::pair<std::size_t,std::size_t> lineInfo( const char* begin, const char* at)
{
	std::size_t line = 1, col = 1;
	char const* si = begin;
	for (; si < at; ++si)
	{
		if (*si == '\n')
		{
			++line;
			col=1;
		}
		else
		{
			++col;
		}
	}
	return std::pair<std::size_t,std::size_t>(line,col);
}

cJSON* strus::parseJsonTree( const std::string& content)
{
	cJSON_Context ctx;
	cJSON* tree = cJSON_Parse( &ctx, content.c_str());
	if (!tree)
	{
		if (!ctx.errorptr) throw std::bad_alloc();
		std::pair<std::size_t,std::size_t> li = lineInfo( content.c_str(), ctx.errorptr);

		std::string err( ctx.errorptr);
		throw strus::runtime_error( _TXT( "error in JSON at line %u, column %u: %s"), (unsigned int)li.first, (unsigned int)li.second, err.c_str());
	}
	return tree;
}

