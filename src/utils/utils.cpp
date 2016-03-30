/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "private/utils.hpp"
#include "private/internationalization.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/stat.h>

using namespace strus;
using namespace strus::utils;

std::string utils::tolower( const std::string& val)
{
	return boost::algorithm::to_lower_copy( val);
}

std::string utils::trim( const std::string& val)
{
	return boost::algorithm::trim_copy( val);
}

bool utils::caseInsensitiveEquals( const std::string& val1, const std::string& val2)
{
	return boost::algorithm::iequals( val1, val2);
}

bool utils::caseInsensitiveStartsWith( const std::string& val, const std::string& prefix)
{
	return boost::algorithm::istarts_with( val, prefix);
}

int utils::toint( const std::string& val)
{
	try
	{
		return boost::lexical_cast<int>( val);
	}
	catch (const boost::bad_lexical_cast& err)
	{
		throw strus::runtime_error( _TXT("failed to convert string '%s' to integer: %s"), val.c_str(), err.what());
	}
}

int utils::todouble( const std::string& val)
{
	try
	{
		return boost::lexical_cast<double>( val);
	}
	catch (const boost::bad_lexical_cast& err)
	{
		throw strus::runtime_error( _TXT( "failed to convert string '%s' to double precision floating point number: %s"), val.c_str(), err.what());
	}
}

std::string utils::tostring( int val)
{
	try
	{
		return boost::lexical_cast<std::string>( val);
	}
	catch (...)
	{
		throw strus::runtime_error( _TXT("failed to convert number to string (out of memory)"));
	}
}

bool utils::isFile( const std::string& path)
{
	struct stat s;
	if (::stat( path.c_str(), &s) == 0)
	{
		if( s.st_mode & S_IFREG )
		{
			return true;
		}
	}
	return false;
}

