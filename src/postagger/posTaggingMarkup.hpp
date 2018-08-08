/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Implements XML markup of a content with POS tagging according to a fixed description
/// \file posTaggingMarkup.hpp
#ifndef _STRUS_ANALYZER_POS_TAGGING_MARKUP_HPP_INCLUDED
#define _STRUS_ANALYZER_POS_TAGGING_MARKUP_HPP_INCLUDED
#include <string>
#include <map>
#include <vector>
#include <utility>
#include "strus/base/symbolTable.hpp"

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;


/// \brief Defines an item describing the statistics in a collection
class PosTaggingMarkup
{
public:
	PosTaggingMarkup( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_)
		,m_symtab(errorhnd_)
		,m_seqtab(errorhnd_)
		,m_allocator(errorhnd_)
		,m_typedescrlist()
		,m_tagmap()
		,m_tagar()
		,m_rootidx(0){}

public:

	void addInfo( const std::vector<Element>& sentence);

	std::string process( const std::string& xmlcontent);

private:
	struct Node
	{
		int tag;
		int value;
		int next;
		int chld;
	
#if __cplusplus >= 201103L
		Node( const Node&& ) = default;
		Node& operator= ( const Node&& ) = default;
#endif
		Node( const Node& o)
			:value(o.value),next(o.next),chld(o.chld){}
		Node( int value_)
			:value(value_),next(0),chld(0){}
	};

	int getOrCreateTagIdx( const std::string& tagnam);
	int getTagIdx( const std::string& tagnam) const;
	const std::string& getTagName( int tag) const;
	int getOrCreateValueIdx( const std::string& val);
	int getValueIdx( const std::string& val) const;
	const char* getValue( int value) const;

	const char* getTypeList( const std::vector<Element>& sentence);
	const char* getValueList( const std::vector<Element>& sentence);
	std::vector<const char*> getTagList( const std::vector<std::string>& sentence) const;
	std::string processContent( const char* src, int srclen) const;
	static std::vector<std::string> tokenize( const char* src, int srclen);

	std::vector<const char*> m_typedescrlist;
	TagMap m_typemap;
	std::vector<std::string> m_tagar;
	int m_rootidx;

private:
#if __cplusplus >= 201103L
	PosTaggingMarkup( const PosTaggingMarkup& ) = delete;
	PosTaggingMarkup& operator= ( const PosTaggingMarkup& ) = delete;
#else
	PosTaggingMarkup( const PosTaggingMarkup&)  :m_symtab(o.m_errorhnd){} ///... non copyable
#endif
};

}//namespace
#endif



