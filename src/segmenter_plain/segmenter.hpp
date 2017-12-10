/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_PLAIN_HPP_INCLUDED
#define _STRUS_SEGMENTER_PLAIN_HPP_INCLUDED
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include <string>
#include <set>

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

	virtual SegmenterContextInterface* createContext( const analyzer::DocumentClass& dclass) const;
	virtual SegmenterMarkupContextInterface* createMarkupContext( const analyzer::DocumentClass& dclass, const std::string& content) const;

private:
	ErrorBufferInterface* m_errorhnd;
	std::set<int> m_segids;
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
		return "text/plain";
	}

	virtual SegmenterInstanceInterface* createInstance( const analyzer::SegmenterOptions& opts) const;
	virtual const char* getDescription() const;

private:
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif
