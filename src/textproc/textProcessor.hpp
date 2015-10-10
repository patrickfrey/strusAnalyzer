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
#ifndef _STRUS_ANALYZER_TEXT_PROCESSOR_HPP_INCLUDED
#define _STRUS_ANALYZER_TEXT_PROCESSOR_HPP_INCLUDED
#include "strus/textProcessorInterface.hpp"
#include <map>
#include <vector>

namespace strus {

/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

class TextProcessor
	:public TextProcessorInterface
{
public:
	explicit TextProcessor( AnalyzerErrorBufferInterface* errorhnd);
	virtual ~TextProcessor();

	virtual void addResourcePath( const std::string& path);
	virtual std::string getResourcePath( const std::string& filename) const;

	virtual const TokenizerFunctionInterface* getTokenizer( const std::string& name) const;

	virtual const NormalizerFunctionInterface* getNormalizer( const std::string& name) const;

	virtual const AggregatorFunctionInterface* getAggregator( const std::string& name) const;

	virtual bool detectDocumentClass( DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize) const;

	virtual void defineDocumentClassDetector( DocumentClassDetectorInterface* detector);

	virtual void defineTokenizer( const std::string& name, TokenizerFunctionInterface* tokenizer);

	virtual void defineNormalizer( const std::string& name, NormalizerFunctionInterface* normalizer);

	virtual void defineAggregator( const std::string& name, AggregatorFunctionInterface* statfunc);

private:
	std::map<std::string,TokenizerFunctionInterface*> m_tokenizer_map;
	std::map<std::string,NormalizerFunctionInterface*> m_normalizer_map;
	std::map<std::string,AggregatorFunctionInterface*> m_aggregator_map;
	std::vector<std::string> m_resourcePaths;
	std::vector<DocumentClassDetectorInterface*> m_detectors;
	AnalyzerErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

