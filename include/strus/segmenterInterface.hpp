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
#ifndef _STRUS_SEGMENTER_INTERFACE_HPP_INCLUDED
#define _STRUS_SEGMENTER_INTERFACE_HPP_INCLUDED
#include <utility>
#include <string>

namespace strus
{
/// \brief Forward declaration
class SegmenterInstanceInterface;

/// \brief Defines a program for splitting a source text it into chunks with an id correspoding to a selecting expression.
class SegmenterInterface
{
public:
	/// \brief Destructor
	virtual ~SegmenterInterface(){}

	/// \brief Defines an expression for selecting chunks from a document
	/// \param[in] id identifier of the chunks that match to expression
	/// \param[in] expression expression for selecting chunks
	virtual void defineSelectorExpression( int id, const std::string& expression)=0;

	/// \enum SelectorType
	/// \brief Classification of selector expressions that determine how positions are assigned to terms
	/// \remark The motivation of defining a selector type is to classify elements that are just markup that should influence positions of real terms.
	enum SelectorType
	{
		Content,			///< An element in the document that gets an own position assigned
		AnnotationSuccessor,		///< An element in the document that gets the position of the succeding content element assigned
		AnnotationPredecessor		///< An element in the document that gets the position of the preceding content element assigned
	};
	static const char* selectorTypeName( SelectorType t)
	{
		static const char* ar[] = {"Content","AnnotationSuccessor","AnnotationPredecessor"};
		return ar[t];
	}

	/// \brief Evaluate the selector type of a feature defined by id
	/// \param[in] id id of the selector as assigned with defineSelectorExpression(int,const std::string&)
	/// \remark Do not call this function before all selector expressions are defined. It works but might lead to a wrong classification. The classification is influenced by other expressions
	/// \return the selector type
	virtual SelectorType getSelectorType( int id) const=0;

	/// \brief Creates an instance of the segmenter
	/// \param[in] source pointer to source. Expected to be valid the whole life time of the instance created
	/// \return the segmenter object to be desposed with delete by the caller
	virtual SegmenterInstanceInterface* createInstance( const std::string& source) const=0;
};

}//namespace
#endif

