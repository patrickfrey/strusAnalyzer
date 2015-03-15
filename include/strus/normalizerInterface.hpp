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
#ifndef _STRUS_ANALYZER_NORMALIZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_INTERFACE_HPP_INCLUDED
#include <vector>
#include <string>

namespace strus
{
/// \brief Forward declaration
class TextProcessorInterface;

class NormalizerInterface
{
public:
	/// \brief Destructor
	virtual ~NormalizerInterface(){}

	/// \brief Normalizer argument base class
	class Argument
	{
	public:
		/// \brief Destructor
		virtual ~Argument(){}
	};

	/// \brief Normalizer context base class
	class Context
	{
	public:
		/// \brief Destructor
		virtual ~Context(){}
	};

	/// \brief Create the arguments needed for normalization
	/// \param[in] textproc text processor for resolving resources
	/// \param[in] src arguments for the normalization as list of strings
	/// \return the argument object to be desposed by the caller with delete if not NULL
	virtual Argument* createArgument( const TextProcessorInterface* textproc, const std::vector<std::string>&) const	{return 0;}

	/// \brief Create the context object needed for normalization
	/// \param[in] arg the normalizer arguments
	/// \return the context object to be desposed by the caller with delete if not NULL
	virtual Context* createContext( const Argument*) const				{return 0;}

	/// \brief Normalization of a token, transforming it into the unit that is stored or retrieved as such in the storage
	/// \param[in] ctx context object for normalization, if needed. created with createContext(const std::string&)const 
	/// \param[in] src start of the source chunk
	/// \param[in] srcsize size of the source chunk
	virtual std::string normalize(
			Context* ctx,
			const char* src,
			std::size_t srcsize) const=0;
};

}//namespace
#endif

