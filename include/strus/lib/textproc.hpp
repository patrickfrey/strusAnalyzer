/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2015 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
/// \brief Exported functions of the strus text processor library (container for all types of functions needed for document and query analysis)
/// \file textproc.hpp
#ifndef _STRUS_ANALYZER_TEXT_PROCESSOR_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_TEXT_PROCESSOR_LIB_HPP_INCLUDED
#include <string>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class TextProcessorInterface;
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

/// \brief Create a text processor
/// \return the constructed text processor
TextProcessorInterface* createTextProcessor( AnalyzerErrorBufferInterface* errorhnd);

}//namespace
#endif

