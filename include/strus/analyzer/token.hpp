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
#ifndef _STRUS_ANALYZER_TOKENIZER_TOKEN_HPP_INCLUDED
#define _STRUS_ANALYZER_TOKENIZER_TOKEN_HPP_INCLUDED
#include <string>

namespace strus {
namespace analyzer {

struct Token
{
	unsigned int docpos;	///< logical byte position in the document. This value is used to assign the term position, that is not the byte position but a number taken from the enumeration of all distinct feature byte postions
	unsigned int strpos;	///< start byte position of the token string in the original document segment
	unsigned int strsize;	///< byte size of the token string in the original document segment

	Token( unsigned int docpos_, unsigned int strpos_, unsigned int strsize_)
		:docpos(docpos_),strpos(strpos_),strsize(strsize_){}
	Token( const Token& o)
		:docpos(o.docpos),strpos(o.strpos),strsize(o.strsize){}
};

}}//namespace
#endif

