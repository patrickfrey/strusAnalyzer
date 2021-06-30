/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_TEXTWOLF_HPP_INCLUDED
#define _STRUS_SEGMENTER_TEXTWOLF_HPP_INCLUDED
#include "strus/segmenterInterface.hpp"
#include "strus/contentIteratorInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "private/xpathAutomaton.hpp"
#include <string>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

class TextwolfSegmenterInstance
	:public SegmenterInstanceInterface
{
public:
	TextwolfSegmenterInstance( ErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}
	virtual ~TextwolfSegmenterInstance(){}

	virtual void defineSelectorExpression( int id, const std::string& expression);
	virtual void defineSubSection( int startId, int endId, const std::string& expression);

	virtual SegmenterContextInterface* createContext( const analyzer::DocumentClass& dclass) const;
	virtual SegmenterMarkupContextInterface* createMarkupContext( const analyzer::DocumentClass& dclass, const std::string& content) const;

	virtual const char* name() const	{return "textwolf";}
	virtual StructView view() const;

private:
	void addExpression( int id, const std::string& expression);

private:
	XPathAutomaton m_automaton;
	ErrorBufferInterface* m_errorhnd;
};


class TextwolfSegmenter
	:public SegmenterInterface
{
public:
	explicit TextwolfSegmenter( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}
	virtual ~TextwolfSegmenter(){}

	virtual const char* mimeType() const
	{
		return "application/xml";
	}

	virtual SegmenterInstanceInterface* createInstance( const analyzer::SegmenterOptions& opts) const;

	virtual ContentIteratorInterface* createContentIterator(
			const char* content,
			std::size_t contentsize,
			const std::vector<std::string>& attributes,
			const std::vector<std::string>& expressions,
			const analyzer::DocumentClass& dclass,
			const analyzer::SegmenterOptions &opts=analyzer::SegmenterOptions()) const;

	virtual const char* name() const	{return "textwolf";}
	virtual StructView view() const;

private:
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

