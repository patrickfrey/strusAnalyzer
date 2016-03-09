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

