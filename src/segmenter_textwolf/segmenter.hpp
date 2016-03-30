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
#include "strus/segmenterInstanceInterface.hpp"
#include "textwolf/xmlpathautomatonparse.hpp"
#include "strus/documentClass.hpp"
#include <string>

namespace strus
{
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

class SegmenterInstance
	:public SegmenterInstanceInterface
{
public:
	SegmenterInstance( AnalyzerErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}
	virtual ~SegmenterInstance(){}

	virtual void defineSelectorExpression( int id, const std::string& expression);
	virtual void defineSubSection( int startId, int endId, const std::string& expression);

	virtual SegmenterContextInterface* createContext( const DocumentClass& dclass) const;

private:
	void addExpression( int id, const std::string& expression);

private:
	typedef textwolf::XMLPathSelectAutomatonParser<> Automaton;
	Automaton m_automaton;
	AnalyzerErrorBufferInterface* m_errorhnd;
};


class Segmenter
	:public SegmenterInterface
{
public:
	explicit Segmenter( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}
	virtual ~Segmenter(){}

	virtual const char* mimeType() const
	{
		return "text/xml";
	}

	virtual SegmenterInstanceInterface* createInstance() const;

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

