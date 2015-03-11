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
#include "documentAnalyzer.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/normalizerInterface.hpp"
#include "strus/tokenizerInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "private/utils.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <set>
#include <map>

using namespace strus;

DocumentAnalyzer::DocumentAnalyzer(
		const TextProcessorInterface* textProcessor_,
		SegmenterInterface* segmenter_)
	:m_textProcessor(textProcessor_),m_segmenter(segmenter_){}


const DocumentAnalyzer::FeatureConfig& DocumentAnalyzer::featureConfig( int featidx) const
{
	if (featidx <= 0 || (std::size_t)featidx > m_featurear.size())
	{
		throw std::runtime_error( "internal: unknown index of feature");
	}
	return m_featurear[ featidx-1];
}

enum {MaxNofFeatures=(1<<24)-1, EndOfSubDocument=(1<<24), OfsSubDocument=(1<<24)+1, MaxNofSubDocuments=(1<<7)};

void DocumentAnalyzer::defineFeature(
	FeatureClass featureClass,
	const std::string& name,
	const std::string& expression,
	const TokenizerConfig& tokenizer,
	const NormalizerConfig& normalizer,
	const FeatureOptions& options)
{
	const TokenizerInterface* tk = m_textProcessor->getTokenizer( tokenizer.name());
	const NormalizerInterface* nm = m_textProcessor->getNormalizer( normalizer.name());

	if (tk->concatBeforeTokenize())
	{
		if (options.positionBind() != FeatureOptions::BindContent)
		{
			throw std::runtime_error( "illegal definition of a feature that has a tokenizer processing the content concatenated with positions bound to other features");
		}
	}
	utils::SharedPtr<TokenizerInterface::Argument> tkarg( tk->createArgument( tokenizer.arguments()));
	if (!tkarg.get() && !tokenizer.arguments().empty())
	{
		throw std::runtime_error( std::string( "no arguments expected for tokenizer '") + tokenizer.name() + "'");
	}
	utils::SharedPtr<NormalizerInterface::Argument> nmarg( nm->createArgument( normalizer.arguments()));
	if (!nmarg.get() && !normalizer.arguments().empty())
	{
		throw std::runtime_error( std::string( "no arguments expected for normalizer '") + normalizer.name() + "'");
	}

	m_featurear.push_back( FeatureConfig( name, tk, tkarg, nm, nmarg, featureClass, options));
	if (m_featurear.size() >= MaxNofFeatures)
	{
		throw std::runtime_error( "number of features defined exceeds maximum limit");
	}
	m_segmenter->defineSelectorExpression( m_featurear.size(), expression);
}

void DocumentAnalyzer::defineSubDocument(
		const std::string& subDocumentTypeName,
		const std::string& selectexpr)
{
	unsigned int subDocumentType = m_subdoctypear.size();
	m_subdoctypear.push_back( subDocumentTypeName);
	if (subDocumentType >= MaxNofSubDocuments)
	{
		throw std::runtime_error( "too many sub documents defined");
	}
	m_segmenter->defineSubSection( subDocumentType+OfsSubDocument, EndOfSubDocument, selectexpr);
}


ParserContext::ParserContext( const std::vector<DocumentAnalyzer::FeatureConfig>& config)
{
	try
	{
		m_size = config.size();

		m_tokenizerContextAr = (TokenizerInterface::Context**)
				std::calloc( m_size, sizeof( m_tokenizerContextAr[0]));
		m_normalizerContextAr = (NormalizerInterface::Context**)
				std::calloc( m_size, sizeof( m_normalizerContextAr[0]));

		if (!m_tokenizerContextAr || !m_normalizerContextAr)
		{
			throw std::bad_alloc();
		}

		std::vector<DocumentAnalyzer::FeatureConfig>::const_iterator
			ci = config.begin(), ce = config.end();
		for (std::size_t cidx=0; ci != ce; ++ci,++cidx)
		{
			m_tokenizerContextAr[ cidx] = ci->tokenizer()->createContext( ci->tokenizerarg());
			m_normalizerContextAr[ cidx] = ci->normalizer()->createContext( ci->normalizerarg());
		}
	}
	catch (const std::bad_alloc& err)
	{
		cleanup();
		throw err;
	}
}

analyzer::Document DocumentAnalyzer::analyze( const std::string& content) const
{
	analyzer::Document rt;
	std::istringstream input( content);
	return analyze( input);
}

analyzer::Document DocumentAnalyzer::analyze( std::istream& input) const
{
	analyzer::Document rt;
	std::auto_ptr<DocumentAnalyzerInstance>
		analyzerInstance( new DocumentAnalyzerInstance( this, input));
	return analyzerInstance->analyzeNext();
}

DocumentAnalyzerInstanceInterface* DocumentAnalyzer::createDocumentAnalyzerInstance( std::istream& input) const
{
	return new DocumentAnalyzerInstance( this, input);
}

void ParserContext::cleanup()
{
	if (m_tokenizerContextAr)
	{
		for (std::size_t ii=0; ii < m_size; ++ii)
		{
			if (m_tokenizerContextAr[ ii]) delete m_tokenizerContextAr[ ii];
		}
		std::free( m_tokenizerContextAr);
		m_tokenizerContextAr = 0;
	}
	if (m_normalizerContextAr)
	{
		for (std::size_t ii=0; ii < m_size; ++ii)
		{
			if (m_normalizerContextAr[ ii]) delete m_normalizerContextAr[ ii];
		}
		std::free( m_normalizerContextAr);
		m_normalizerContextAr = 0;
	}
}

NormalizerInterface::Context* ParserContext::normalizerContext( int featidx) const
{
	if (featidx <= 0 || (std::size_t)featidx > m_size)
	{
		throw std::runtime_error( "internal: unknown index of feature");
	}
	return m_normalizerContextAr[ featidx-1];
}

TokenizerInterface::Context* ParserContext::tokenizerContext( int featidx) const
{
	if (featidx <= 0 || (std::size_t)featidx > m_size)
	{
		throw std::runtime_error( "internal: unknown index of feature");
	}
	return m_tokenizerContextAr[ featidx-1];
}


/// \brief Map byte offset positions to token occurrence positions:
void DocumentAnalyzerInstance::mapPositions( analyzer::Document& res) const
{
	std::set<unsigned int> pset;
	std::vector<analyzer::Term>::const_iterator ri = m_searchTerms.begin(), re = m_searchTerms.end();
	for (; ri != re; ++ri)
	{
		pset.insert( ri->pos());
	}
	ri = m_forwardTerms.begin(), re = m_forwardTerms.end();
	for (; ri != re; ++ri)
	{
		pset.insert( ri->pos());
	}
	std::map<unsigned int, unsigned int> posmap;
	std::set<unsigned int>::const_iterator pi = pset.begin(), pe = pset.end();
	unsigned int pcnt = 0;
	for (; pi != pe; ++pi)
	{
		posmap[ *pi] = ++pcnt;
	}
	std::size_t posofs = 0;
	if (res.searchIndexTerms().size() && res.searchIndexTerms().back().pos() > posofs)
	{
		posofs = res.searchIndexTerms().back().pos();
	}
	if (res.forwardIndexTerms().size() && res.forwardIndexTerms().back().pos() > posofs)
	{
		posofs = res.forwardIndexTerms().back().pos();
	}
	for (ri = m_searchTerms.begin(), re = m_searchTerms.end(); ri != re; ++ri)
	{
		unsigned int pos = posmap[ ri->pos()];
		res.addSearchIndexTerm( ri->type(), ri->value(), pos + posofs);
	}
	for (ri = m_forwardTerms.begin(), re = m_forwardTerms.end(); ri != re; ++ri)
	{
		unsigned int pos = posmap[ ri->pos()];
		res.addForwardIndexTerm( ri->type(), ri->value(), pos + posofs);
	}
}

void DocumentAnalyzerInstance::processDocumentSegment( analyzer::Document& res, int featidx, std::size_t rel_position, const char* elem, std::size_t elemsize)
{
	TokenizerInterface::Context* tokctx = m_parserContext.tokenizerContext( featidx);
	NormalizerInterface::Context* normctx = m_parserContext.normalizerContext( featidx);

	const DocumentAnalyzer::FeatureConfig& feat = m_analyzer->featureConfig( featidx);
	std::vector<analyzer::Token> tokens = feat.tokenizer()->tokenize( tokctx, elem, elemsize);

	switch (feat.featureClass())
	{
		case DocumentAnalyzer::FeatMetaData:
		{
			if (!tokens.empty())
			{
				std::string valstr(
					feat.normalizer()->normalize(
						normctx, elem + tokens[0].strpos, tokens[0].strsize));
				res.setMetaData( feat.name(), valstr);
			}
			if (tokens.size() > 1)
			{
				throw std::runtime_error( "metadata feature tokenized to to more than one part");
			}
			break;
		}
		case DocumentAnalyzer::FeatAttribute:
		{
			std::vector<analyzer::Token>::const_iterator
				ti = tokens.begin(), te = tokens.end();
			for (; ti != te; ++ti)
			{
				std::string valstr(
					feat.normalizer()->normalize(
						normctx, elem + ti->strpos, ti->strsize));
				res.setAttribute( feat.name(), valstr);
			}
			break;
		}
		case DocumentAnalyzer::FeatSearchIndexTerm:
		{
			std::vector<analyzer::Token>::const_iterator
				ti = tokens.begin(), te = tokens.end();
			for (; ti != te; ++ti)
			{
				std::string valstr(
					feat.normalizer()->normalize(
						normctx, elem + ti->strpos, ti->strsize));
				m_searchTerms.push_back(
					analyzer::Term( feat.name(), valstr, rel_position + ti->docpos));
			}
			break;
		}
		case DocumentAnalyzer::FeatForwardIndexTerm:
		{
			std::vector<analyzer::Token>::const_iterator
				ti = tokens.begin(), te = tokens.end();
			for (; ti != te; ++ti)
			{
				std::string valstr(
					feat.normalizer()->normalize(
						normctx, elem + ti->strpos, ti->strsize));
				m_forwardTerms.push_back(
					analyzer::Term( feat.name(), valstr, rel_position + ti->docpos));
			}
			break;
		}
	}
}


struct Segment
{
	unsigned int pos;
	unsigned int size;

	Segment( unsigned int pos_, unsigned int size_)
		:pos(pos_),size(size_){}
	Segment( const Segment& o)
		:pos(o.pos),size(o.size){}
};

void DocumentAnalyzerInstance::processConcatenated(
		analyzer::Document& res)
{
	ConcatenatedMap::const_iterator
		ci = m_concatenatedMap.begin(),
		ce = m_concatenatedMap.end();

	for (; ci != ce; ++ci)
	{
		processDocumentSegment(
			res, ci->first, ci->second.position, ci->second.content.c_str(), ci->second.content.size());
	}
}

void DocumentAnalyzerInstance::concatDocumentSegment( int featidx, std::size_t rel_position, const char* elem, std::size_t elemsize)
{
	ConcatenatedMap::iterator ci = m_concatenatedMap.find( featidx);
	if (ci == m_concatenatedMap.end())
	{
		m_concatenatedMap[ featidx]
			= Chunk( rel_position, std::string( elem, elemsize));
	}
	else
	{
		Chunk& cm = m_concatenatedMap[ featidx];
		std::size_t newlen = rel_position - cm.position;
		cm.content.resize( newlen, ' ');
		cm.content.append( elem, elemsize);
	}
}

void DocumentAnalyzerInstance::clearTermMaps()
{
	m_concatenatedMap.clear();
	m_searchTerms.clear();
	m_forwardTerms.clear();
}

struct SuccPositionChunk
{
	SuccPositionChunk( int featidx_, const char* elem_, std::size_t elemsize_)
		:featidx(featidx_),elem(elem_,elemsize_){}
	SuccPositionChunk( const SuccPositionChunk& o)
		:featidx(o.featidx),elem(o.elem){}
	int featidx;
	std::string elem;
};

analyzer::Document DocumentAnalyzerInstance::analyzeNext()
{
	if (m_subdocstack.empty())
	{
		throw std::runtime_error( "internal: called analyzeNext after EOF");
	}
	analyzer::Document rt = m_subdocstack.back();
	m_subdocstack.pop_back();
	const char* elem = 0;
	std::size_t elemsize = 0;
	int featidx = 0;
	SegmenterPosition last_position = 0;
	SegmenterPosition curr_position = 0;
	SegmenterPosition start_position = 0;
	std::vector<SuccPositionChunk> succChunks;

	// [1] Scan the document and push the normalized tokenization of the elements to the result:
	while (m_segmenter->getNext( featidx, curr_position, elem, elemsize))
	{
		try
		{
			if (featidx >= EndOfSubDocument)
			{
				//... start or end of document marker
				if (featidx == EndOfSubDocument)
				{
					//... end of sub document -> out of loop and return document
					break;
				}
				else
				{
					// process chunks bound to successor not processed yet (without successor):
					std::vector<SuccPositionChunk>::const_iterator
						si = succChunks.begin(), se = succChunks.end();
					std::size_t rel_position
						= (std::size_t)(curr_position - start_position);
					for (; si != se; ++si)
					{
						processDocumentSegment( rt, si->featidx, rel_position, si->elem.c_str(), si->elem.size());
					}
					succChunks.clear();

					// process what is left to process for the current sub document:
					processConcatenated( rt);
					mapPositions( rt);
					clearTermMaps();

					// create new sub document:
					m_subdocstack.push_back( rt);
					rt.setSubDocumentTypeName( m_analyzer->m_subdoctypear[ featidx-OfsSubDocument]);
					start_position = curr_position;
				}
			}
			else
			{
				const DocumentAnalyzer::FeatureConfig& feat = m_analyzer->featureConfig( featidx);

				if (feat.tokenizer()->concatBeforeTokenize())
				{
					// concat chunks that need to be concatenated before tokenization:
					std::size_t rel_position = (std::size_t)(curr_position - start_position);
					concatDocumentSegment( featidx, rel_position, elem, elemsize);
					continue;
				}
				else
				{
					switch (feat.options().positionBind())
					{
						case DocumentAnalyzerInterface::FeatureOptions::BindContent:
						{
							// process chunks bound to successor (this chunk):
							std::vector<SuccPositionChunk>::const_iterator
								si = succChunks.begin(), se = succChunks.end();
							std::size_t rel_position
								= (std::size_t)(curr_position - start_position);
							for (; si != se; ++si)
							{
								processDocumentSegment( rt, si->featidx, rel_position, si->elem.c_str(), si->elem.size());
							}
							succChunks.clear();

							// process this chunk:
							last_position = curr_position;
							processDocumentSegment( rt, featidx, rel_position, elem, elemsize);
							break;
						}
						case DocumentAnalyzerInterface::FeatureOptions::BindSuccessor:
						{
							succChunks.push_back( SuccPositionChunk( featidx, elem, elemsize));
							break;
						}
						case DocumentAnalyzerInterface::FeatureOptions::BindPredecessor:
						{
							std::size_t rel_position
								= (std::size_t)(last_position - start_position);
							processDocumentSegment( rt, featidx, rel_position, elem, elemsize);
							break;
						}
					}
				}
			}
		}
		catch (const std::runtime_error& err)
		{
			throw std::runtime_error( std::string( "error in analyze when processing chunk (") + std::string( elem, elemsize) + "): " + err.what());
		}
	}
	// process chunks bound to successor not processed yet (without successor):
	std::vector<SuccPositionChunk>::const_iterator
		si = succChunks.begin(), se = succChunks.end();
	std::size_t rel_position
		= (std::size_t)(curr_position - start_position);
	for (; si != se; ++si)
	{
		processDocumentSegment( rt, si->featidx, rel_position, si->elem.c_str(), si->elem.size());
	}

	// process concatenated chunks:
	processConcatenated( rt);

	// create real positions for output:
	mapPositions( rt);
	clearTermMaps();
	return rt;
}

