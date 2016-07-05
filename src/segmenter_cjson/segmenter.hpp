/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_CJSON_HPP_INCLUDED
#define _STRUS_SEGMENTER_CJSON_HPP_INCLUDED
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/documentClass.hpp"
#include "private/xpathAutomaton.hpp"
#include <string>

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class SegmenterContextInterface;
/// \brief Forward declaration
class SegmenterMarkupContextInterface;

class SegmenterInstance
	:public SegmenterInstanceInterface
{
public:
	SegmenterInstance( ErrorBufferInterface* errorhnd)
		:m_errorhnd(errorhnd){}
	virtual ~SegmenterInstance(){}

	virtual void defineSelectorExpression( int id, const std::string& expression);
	virtual void defineSubSection( int startId, int endId, const std::string& expression);

	virtual SegmenterContextInterface* createContext( const DocumentClass& dclass) const;
	virtual SegmenterMarkupContextInterface* createMarkupContext( const DocumentClass& dclass, const std::string& content) const;

private:
	void addExpression( int id, const std::string& expression);

private:
	XPathAutomaton m_automaton;
	ErrorBufferInterface* m_errorhnd;
};


class Segmenter
	:public SegmenterInterface
{
public:
	explicit Segmenter( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}
	virtual ~Segmenter(){}

	virtual const char* mimeType() const
	{
		return "application/json";
	}

	virtual SegmenterInstanceInterface* createInstance( const SegmenterOptions& opts) const;

private:
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

