/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the data built by a POS tagger
/// \file posTaggerData.hpp
#ifndef _STRUS_ANALYZER_POS_TAGGER_DATA_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_ANALYZER_POS_TAGGER_DATA_IMPLEMENTATION_HPP_INCLUDED
#include "strus/posTaggerDataInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/base/symbolTable.hpp"
#include <string>
#include <vector>
#include <map>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceContextInterface;

/// \brief Implementation of the data built by a POS tagger
class PosTaggerData
	:public PosTaggerDataInterface
{
public:
	/// \param[in] tokenizer_ tokenizer function to use (passed with ownership)
	/// \param[in] errorhnd_ error buffer interface
	PosTaggerData( TokenizerFunctionInstanceInterface* tokenizer_, ErrorBufferInterface* errorhnd_);
	virtual ~PosTaggerData();

	virtual void defineTag( const std::string& type, const std::string& tag);

	virtual void insert( int docno, const std::vector<Element>& elements);

	void markupSegment( TokenMarkupContextInterface* markupContext, int docno, int& docitr, const SegmenterPosition& segmentpos, const char* segmentptr, std::size_t segmentsize) const;

private:
	typedef std::map<std::string,std::string> TypeTagMap;

	struct TagAssignment
	{
		int value;
		const char* tag;

		TagAssignment( int value_, const char* tag_)
			:value(value_),tag(tag_){}
		TagAssignment( const TagAssignment& o)
			:value(o.value),tag(o.tag){}
	};
	struct DocAssignment
	{
		std::vector<TagAssignment> ar;

		DocAssignment()
			:ar(){}
		DocAssignment( const DocAssignment& o)
			:ar(o.ar){}
	};

private:
	std::vector<analyzer::Token> tokenize( const char* src, std::size_t srcsize) const;
	std::vector<Element> tokenize( const std::vector<Element>& sequence) const;

private:
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
	TokenizerFunctionInstanceInterface* m_tokenizer;
	TypeTagMap m_typeTagMap;
	std::vector<std::string> m_typeList;
	std::vector<DocAssignment> m_docs;
	std::map<int,int> m_docnoDocMap;
	SymbolTable m_symtab;
};

}//namespace
#endif

