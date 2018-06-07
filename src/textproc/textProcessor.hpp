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
class ErrorBufferInterface;
/// \brief Forward declaration
class FileLocatorInterface;

class TextProcessor
	:public TextProcessorInterface
{
public:
	explicit TextProcessor( const FileLocatorInterface* filelocator_, ErrorBufferInterface* errorhnd_);
	virtual ~TextProcessor();

	virtual std::string getResourceFilePath( const std::string& filename) const;

	virtual const SegmenterInterface* getSegmenterByName( const std::string& segmenterName) const;

	virtual const SegmenterInterface* getSegmenterByMimeType( const std::string& mimetype) const;

	virtual analyzer::SegmenterOptions getSegmenterOptions( const std::string& scheme) const;

	virtual const TokenizerFunctionInterface* getTokenizer( const std::string& name) const;

	virtual const NormalizerFunctionInterface* getNormalizer( const std::string& name) const;

	virtual const AggregatorFunctionInterface* getAggregator( const std::string& name) const;

	virtual const PatternLexerInterface* getPatternLexer( const std::string& name) const;

	virtual const PatternMatcherInterface* getPatternMatcher( const std::string& name) const;

	virtual const PatternTermFeederInterface* getPatternTermFeeder() const;

	virtual bool detectDocumentClass( analyzer::DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize, bool isComplete) const;

	virtual void defineDocumentClassDetector( DocumentClassDetectorInterface* detector);

	virtual void defineSegmenter( const std::string& name, SegmenterInterface* segmenter);

	virtual void defineSegmenterOptions( const std::string& scheme, const analyzer::SegmenterOptions& options);
	
	virtual void defineTokenizer( const std::string& name, TokenizerFunctionInterface* tokenizer);

	virtual void defineNormalizer( const std::string& name, NormalizerFunctionInterface* normalizer);

	virtual void defineAggregator( const std::string& name, AggregatorFunctionInterface* statfunc);

	virtual void definePatternLexer( const std::string& name, PatternLexerInterface* lexer);

	virtual void definePatternMatcher( const std::string& name, PatternMatcherInterface* matcher);

	virtual std::vector<std::string> getFunctionList( const FunctionType& type) const;

private:
	ErrorBufferInterface* m_errorhnd;
	const FileLocatorInterface* m_filelocator;
	std::map<std::string,SegmenterInterface*> m_segmenterMap;		///< map of defined document segmenters (key is segmenter name)
	std::map<std::string,SegmenterInterface*> m_mimeSegmenterMap;		///< map of defined document segmenters (key is MIME type)
	std::map<std::string,analyzer::SegmenterOptions> m_schemeSegmenterOptions_map;
	std::map<std::string,TokenizerFunctionInterface*> m_tokenizer_map;
	std::map<std::string,NormalizerFunctionInterface*> m_normalizer_map;
	std::map<std::string,AggregatorFunctionInterface*> m_aggregator_map;
	std::map<std::string,PatternLexerInterface*> m_patternlexer_map;
	std::map<std::string,PatternMatcherInterface*> m_patternmatcher_map;
	PatternTermFeederInterface* m_patterntermfeeder;
	std::vector<std::string> m_resourcePaths;
	std::vector<DocumentClassDetectorInterface*> m_detectors;
};

}//namespace
#endif

