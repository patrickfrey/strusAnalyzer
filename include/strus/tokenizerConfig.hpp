/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#ifndef _STRUS_ANALYZER_TOKENIZER_CONFIG_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_CONFIG_HPP_INCLUDED
#include <string>
#include <vector>

namespace strus {

class TokenizerConfig
{
public:
	TokenizerConfig()
		:m_name(),m_arguments(){}
	TokenizerConfig( const TokenizerConfig& o)
		:m_name(o.m_name),m_arguments(o.m_arguments){}
	TokenizerConfig( const std::string& name_, const std::vector<std::string>& arguments_)
		:m_name(name_),m_arguments(arguments_){}
	TokenizerConfig( const std::string& name_)
		:m_name(name_),m_arguments(){}

	const std::string& name() const				{return m_name;}
	const std::vector<std::string>& arguments() const	{return m_arguments;}

private:
	std::string m_name;
	std::vector<std::string> m_arguments;
};

}//namespace
#endif

