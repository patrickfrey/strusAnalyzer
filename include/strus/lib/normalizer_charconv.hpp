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
/// \brief Exported functions of the strus analyzer character conversion normalization function library
/// \file normalizer_charconv.hpp
#ifndef _STRUS_ANALYZER_NORMALIZER_CHARACTER_CONVERSIONS_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_NORMALIZER_CHARACTER_CONVERSIONS_LIB_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class NormalizerFunctionInterface;
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

/// \brief Get the normalizer that returns the lower case of the input as result
/// \return the normalization function
NormalizerFunctionInterface* createNormalizer_lowercase( AnalyzerErrorBufferInterface* errorhnd);
/// \brief Get the normalizer that returns the upper case of the input as result
/// \return the normalization function
NormalizerFunctionInterface* createNormalizer_uppercase( AnalyzerErrorBufferInterface* errorhnd);
/// \brief Get the normalizer that returns the conversion of diacritical characters to ascii of the input as result
/// \return the normalization function
NormalizerFunctionInterface* createNormalizer_convdia( AnalyzerErrorBufferInterface* errorhnd);

}//namespace
#endif

