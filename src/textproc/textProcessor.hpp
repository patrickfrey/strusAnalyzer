/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

	virtual std::vector<std::string> getFunctionList( const FunctionType& type) const;

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

