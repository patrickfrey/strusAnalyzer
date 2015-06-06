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
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "private/utils.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <set>
#include <map>
#include <cstring>

using namespace strus;

DocumentAnalyzer::DocumentAnalyzer( SegmenterInterface* segmenter_)
	:m_segmenter(segmenter_){}


const DocumentAnalyzer::FeatureConfig& DocumentAnalyzer::featureConfig( int featidx) const
{
	if (featidx <= 0 || (std::size_t)featidx > m_featurear.size())
	{
		throw std::runtime_error( "internal: unknown index of feature");
	}
	return m_featurear[ featidx-1];
}

enum {MaxNofFeatures=(1<<24)-1, EndOfSubDocument=(1<<24), OfsSubDocument=(1<<24)+1, MaxNofSubDocuments=(1<<7)};

DocumentAnalyzer::FeatureConfig::FeatureConfig(
		const std::string& name_,
		TokenizerFunctionInstanceInterface* tokenizer_,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers_,
		FeatureClass featureClass_,
		const FeatureOptions& options_)
	:m_name(name_)
	,m_tokenizer(tokenizer_)
	,m_featureClass(featureClass_)
	,m_options(options_)
{
	if (m_tokenizer->concatBeforeTokenize())
	{
		if (m_options.positionBind() != FeatureOptions::BindContent)
		{
			throw std::runtime_error( "illegal definition of a feature that has a tokenizer processing the content concatenated with positions bound to other features");
		}
	}
	std::vector<NormalizerFunctionInstanceInterface*>::const_iterator
		ci = normalizers_.begin(), ce = normalizers_.end();
	for (; ci != ce; ++ci)
	{
		m_normalizerlist.push_back( *ci);
	}
}

void DocumentAnalyzer::defineFeature(
		FeatureClass featureClass,
		const std::string& name,
		const std::string& expression,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const FeatureOptions& options)
{
	m_featurear.push_back( FeatureConfig( name, tokenizer, normalizers, featureClass, options));
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


ParserContext::FeatureContext::FeatureContext( const DocumentAnalyzer::FeatureConfig& config)
	:m_config(&config)
	,m_tokenizerContext(config.tokenizer()->createFunctionContext())
{
	std::vector<DocumentAnalyzer::FeatureConfig::NormalizerReference>::const_iterator
		ni = config.normalizerlist().begin(),
		ne = config.normalizerlist().end();
	
	for (; ni != ne; ++ni)
	{
		m_normalizerContextAr.push_back( (*ni)->createFunctionContext());
	}
}

std::string ParserContext::FeatureContext::normalize( char const* tok, std::size_t toksize)
{
	NormalizerFunctionContextArray::iterator
		ci = m_normalizerContextAr.begin(),
		ce = m_normalizerContextAr.end();

	std::string rt;
	std::string origstr;
	for (; ci != ce; ++ci)
	{
		rt = (*ci)->normalize( tok, toksize);
		if (ci + 1 != ce)
		{
			origstr.swap( rt);
			tok = origstr.c_str();
			toksize = origstr.size();
		}
	}
	return rt;
}

ParserContext::ParserContext( const std::vector<DocumentAnalyzer::FeatureConfig>& config)
{
	std::vector<DocumentAnalyzer::FeatureConfig>::const_iterator
		ci = config.begin(), ce = config.end();
	for (; ci != ce; ++ci)
	{
		m_featureContextAr.push_back( FeatureContext( *ci));
	}
}

analyzer::Document DocumentAnalyzer::analyze( const std::string& content) const
{
	analyzer::Document rt;
	std::auto_ptr<DocumentAnalyzerContext>
		analyzerInstance( new DocumentAnalyzerContext( this));
	analyzerInstance->putInput( content.c_str(), content.size(), true);
	if (!analyzerInstance->analyzeNext( rt))
	{
		throw std::runtime_error( "analyzed content incomplete or empty");
	}
	return rt;
}

DocumentAnalyzerContextInterface* DocumentAnalyzer::createContext() const
{
	return new DocumentAnalyzerContext( this);
}


/// \brief Map byte offset positions to token occurrence positions:
void DocumentAnalyzerContext::mapPositions( analyzer::Document& res) const
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

void DocumentAnalyzerContext::processDocumentSegment( analyzer::Document& res, int featidx, std::size_t rel_position, const char* elem, std::size_t elemsize)
{
	ParserContext::FeatureContext& feat = m_parserContext.featureContext( featidx);

	std::vector<analyzer::Token>
		tokens = feat.m_tokenizerContext->tokenize( elem, elemsize);
	switch (feat.m_config->featureClass())
	{
		case DocumentAnalyzer::FeatMetaData:
		{
			if (!tokens.empty())
			{
				res.setMetaData(
					feat.m_config->name(),
					feat.normalize( elem + tokens[0].strpos, tokens[0].strsize));
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
				res.setAttribute(
					feat.m_config->name(),
					feat.normalize( elem + ti->strpos, ti->strsize));
			}
			break;
		}
		case DocumentAnalyzer::FeatSearchIndexTerm:
		{
			std::vector<analyzer::Token>::const_iterator
				ti = tokens.begin(), te = tokens.end();
			for (; ti != te; ++ti)
			{
				m_searchTerms.push_back(
					analyzer::Term(
						feat.m_config->name(),
						feat.normalize( elem + ti->strpos, ti->strsize),
						rel_position + ti->docpos));
			}
			break;
		}
		case DocumentAnalyzer::FeatForwardIndexTerm:
		{
			std::vector<analyzer::Token>::const_iterator
				ti = tokens.begin(), te = tokens.end();
			for (; ti != te; ++ti)
			{
				m_forwardTerms.push_back(
					analyzer::Term(
						feat.m_config->name(),
						feat.normalize( elem + ti->strpos, ti->strsize),
						rel_position + ti->docpos));
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

void DocumentAnalyzerContext::processConcatenated(
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

void DocumentAnalyzerContext::concatDocumentSegment( int featidx, std::size_t rel_position, const char* elem, std::size_t elemsize)
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

void DocumentAnalyzerContext::clearTermMaps()
{
	m_concatenatedMap.clear();
	m_searchTerms.clear();
	m_forwardTerms.clear();
}

DocumentAnalyzerContext::DocumentAnalyzerContext( const DocumentAnalyzer* analyzer_)
	:m_analyzer(analyzer_)
	,m_segmenter(m_analyzer->m_segmenter->createContext())
	,m_parserContext(analyzer_->m_featurear)
	,m_eof(false)
	,m_last_position(0)
	,m_curr_position(0)
	,m_start_position(0)
{
	m_subdocstack.push_back( analyzer::Document());
}

void DocumentAnalyzerContext::putInput( const char* chunk, std::size_t chunksize, bool eof)
{
	m_segmenter->putInput( chunk, chunksize, eof);
	m_eof = eof;
}

bool DocumentAnalyzerContext::analyzeNext( analyzer::Document& doc)
{
	if (m_subdocstack.empty())
	{
		return false;
	}
	bool have_document = false;
	doc.clear();
	m_subdocstack.back().swap( doc);
	m_subdocstack.pop_back();
	const char* elem = 0;
	std::size_t elemsize = 0;
	int featidx = 0;

	// [1] Scan the document and push the normalized tokenization of the elements to the result:
	while (m_segmenter->getNext( featidx, m_curr_position, elem, elemsize))
	{
		try
		{
			if (featidx >= EndOfSubDocument)
			{
				//... start or end of document marker
				if (featidx == EndOfSubDocument)
				{
					//... end of sub document -> out of loop and return document
					have_document = true;
					break;
				}
				else
				{
					// process chunks bound to successor not processed yet (without successor):
					std::vector<SuccPositionChunk>::const_iterator
						si = m_succChunks.begin(), se = m_succChunks.end();
					std::size_t rel_position
						= (std::size_t)(m_curr_position - m_start_position);
					for (; si != se; ++si)
					{
						processDocumentSegment( doc, si->featidx, rel_position, si->elem.c_str(), si->elem.size());
					}
					m_succChunks.clear();

					// process what is left to process for the current sub document:
					processConcatenated( doc);
					mapPositions( doc);
					clearTermMaps();

					// create new sub document:
					m_subdocstack.push_back( doc);
					doc.setSubDocumentTypeName( m_analyzer->m_subdoctypear[ featidx-OfsSubDocument]);
					m_start_position = m_curr_position;
				}
			}
			else
			{
				const DocumentAnalyzer::FeatureConfig& feat = m_analyzer->featureConfig( featidx);

				if (feat.tokenizer()->concatBeforeTokenize())
				{
					// concat chunks that need to be concatenated before tokenization:
					std::size_t rel_position = (std::size_t)(m_curr_position - m_start_position);
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
								si = m_succChunks.begin(), se = m_succChunks.end();
							std::size_t rel_position
								= (std::size_t)(m_curr_position - m_start_position);
							for (; si != se; ++si)
							{
								processDocumentSegment( doc, si->featidx, rel_position, si->elem.c_str(), si->elem.size());
							}
							m_succChunks.clear();

							// process this chunk:
							m_last_position = m_curr_position;
							processDocumentSegment( doc, featidx, rel_position, elem, elemsize);
							break;
						}
						case DocumentAnalyzerInterface::FeatureOptions::BindSuccessor:
						{
							m_succChunks.push_back( SuccPositionChunk( featidx, elem, elemsize));
							break;
						}
						case DocumentAnalyzerInterface::FeatureOptions::BindPredecessor:
						{
							std::size_t rel_position
								= (std::size_t)(m_last_position - m_start_position);
							processDocumentSegment( doc, featidx, rel_position, elem, elemsize);
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
	if (!m_eof && !have_document)
	{
		m_subdocstack.push_back( analyzer::Document());
		m_subdocstack.back().swap( doc);
		return false;
	}

	// process chunks bound to successor not processed yet (without successor):
	std::vector<SuccPositionChunk>::const_iterator
		si = m_succChunks.begin(), se = m_succChunks.end();
	std::size_t rel_position
		= (std::size_t)(m_curr_position - m_start_position);
	for (; si != se; ++si)
	{
		processDocumentSegment( doc, si->featidx, rel_position, si->elem.c_str(), si->elem.size());
	}

	// process concatenated chunks:
	processConcatenated( doc);

	// create real positions for output:
	mapPositions( doc);
	clearTermMaps();

	if (!doc.attributes().empty()) return true;
	if (!doc.metadata().empty()) return true;
	if (!doc.searchIndexTerms().empty()) return true;
	if (!doc.forwardIndexTerms().empty()) return true;
	return false;
}

