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
#include "strus/analyzerErrorBufferInterface.hpp"
#include "strus/analyzer/token.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <set>
#include <map>
#include <cstring>

using namespace strus;

DocumentAnalyzer::DocumentAnalyzer( const SegmenterInterface* segmenter_, AnalyzerErrorBufferInterface* errorhnd)
	:m_segmenter(segmenter_->createInstance()),m_errorhnd(errorhnd)
{
	if (!m_segmenter)
	{
		throw strus::runtime_error( _TXT("failed to create segmenter context: %s"), errorhnd->fetchError());
	}
}


const DocumentAnalyzer::FeatureConfig& DocumentAnalyzer::featureConfig( int featidx) const
{
	if (featidx <= 0 || (std::size_t)featidx > m_featurear.size())
	{
		throw strus::runtime_error( _TXT("internal: unknown index of feature"));
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
	,m_featureClass(featureClass_)
	,m_options(options_)
{
	if (tokenizer_->concatBeforeTokenize())
	{
		if (m_options.positionBind() != FeatureOptions::BindContent)
		{
			throw strus::runtime_error( _TXT("illegal definition of a feature that has a tokenizer processing the content concatenated with positions bound to other features"));
		}
	}
	// PF:NOTE: The following order of code ensures that if this constructor fails then no tokenizer or normalizer is copied, because otherwise they will be free()d twice:
	m_normalizerlist.reserve( normalizers_.size());
	std::vector<NormalizerFunctionInstanceInterface*>::const_iterator
		ci = normalizers_.begin(), ce = normalizers_.end();
	for (; ci != ce; ++ci)
	{
		m_normalizerlist.push_back( *ci);
	}
	m_tokenizer.reset( tokenizer_);
}

void DocumentAnalyzer::defineFeature(
		FeatureClass featureClass,
		const std::string& name,
		const std::string& expression,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const FeatureOptions& options)
{
	try
	{
		if (m_featurear.size()+1 >= MaxNofFeatures)
		{
			m_errorhnd->report( _TXT("number of features defined exceeds maximum limit"));
		}
		m_segmenter->defineSelectorExpression( m_featurear.size()+1, expression);
		m_featurear.push_back( FeatureConfig( name, tokenizer, normalizers, featureClass, options));
	}
	CATCH_ERROR_MAP( _TXT("error in DocumentAnalyzer::defineFeature: %s"), *m_errorhnd);
}

void DocumentAnalyzer::addSearchIndexFeature(
		const std::string& type,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const FeatureOptions& options)
{
	defineFeature( FeatSearchIndexTerm, type, selectexpr, tokenizer, normalizers, options);
}

void DocumentAnalyzer::addForwardIndexFeature(
		const std::string& type,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const FeatureOptions& options)
{
	defineFeature( FeatForwardIndexTerm, type, selectexpr, tokenizer, normalizers, options);
}

void DocumentAnalyzer::defineMetaData(
		const std::string& fieldname,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	defineFeature( FeatMetaData, fieldname, selectexpr, tokenizer, normalizers, FeatureOptions());
}

void DocumentAnalyzer::defineAttribute(
		const std::string& attribname,
		const std::string& selectexpr,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	defineFeature( FeatAttribute, attribname, selectexpr, tokenizer, normalizers, FeatureOptions());
}

void DocumentAnalyzer::defineAggregatedMetaData(
		const std::string& fieldname,
		AggregatorFunctionInstanceInterface* statfunc)
{
	try
	{
		m_statistics.push_back( StatisticsConfig( fieldname, statfunc));
	}
	CATCH_ERROR_MAP( _TXT("error in DocumentAnalyzer::defineAggregatedMetaData: %s"), *m_errorhnd);
}

void DocumentAnalyzer::defineSubDocument(
		const std::string& subDocumentTypeName,
		const std::string& selectexpr)
{
	try
	{
		unsigned int subDocumentType = m_subdoctypear.size();
		m_subdoctypear.push_back( subDocumentTypeName);
		if (subDocumentType >= MaxNofSubDocuments)
		{
			throw strus::runtime_error( _TXT("too many sub documents defined"));
		}
		m_segmenter->defineSubSection( subDocumentType+OfsSubDocument, EndOfSubDocument, selectexpr);
	}
	CATCH_ERROR_MAP( _TXT("error in DocumentAnalyzer::defineSubDocument: %s"), *m_errorhnd);
}


ParserContext::FeatureContext::FeatureContext( const DocumentAnalyzer::FeatureConfig& config)
	:m_config(&config)
	,m_tokenizerContext(config.tokenizer()->createFunctionContext())
{
	if (!m_tokenizerContext.get())
	{
		throw std::runtime_error( "failed to create tokenizer context");
	}
	std::vector<DocumentAnalyzer::FeatureConfig::NormalizerReference>::const_iterator
		ni = config.normalizerlist().begin(),
		ne = config.normalizerlist().end();
	
	for (; ni != ne; ++ni)
	{
		m_normalizerContextAr.push_back( (*ni)->createFunctionContext());
		if (!m_normalizerContextAr.back().get())
		{
			throw strus::runtime_error( _TXT("failed to create normalizer context"));
		}
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

analyzer::Document DocumentAnalyzer::analyze(
		const std::string& content,
		const DocumentClass& dclass) const
{
	try
	{
		analyzer::Document rt;
		std::auto_ptr<DocumentAnalyzerContext>
			analyzerInstance( new DocumentAnalyzerContext( this, dclass, m_errorhnd));
		analyzerInstance->putInput( content.c_str(), content.size(), true);
		if (!analyzerInstance->analyzeNext( rt))
		{
			throw std::runtime_error( "analyzed content incomplete or empty");
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( "error in DocumentAnalyzer::defineSubDocument: %s", *m_errorhnd, analyzer::Document());
}

DocumentAnalyzerContextInterface* DocumentAnalyzer::createContext( const DocumentClass& dclass) const
{
	try
	{
		return new DocumentAnalyzerContext( this, dclass, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( "error in DocumentAnalyzer::createContext: %s", *m_errorhnd, 0);
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

void DocumentAnalyzerContext::mapStatistics( analyzer::Document& res) const
{
	std::vector<DocumentAnalyzer::StatisticsConfig>::const_iterator
		si = m_analyzer->m_statistics.begin(), se = m_analyzer->m_statistics.end();
	for (; si != se; ++si)
	{
		res.setMetaData( si->name(), si->statfunc()->evaluate( res));
	}
}

void DocumentAnalyzerContext::processDocumentSegment( analyzer::Document& res, int featidx, std::size_t rel_position, const char* elem, std::size_t elemsize, bool samePosition)
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
					utils::todouble( feat.normalize( elem + tokens[0].strpos, tokens[0].strsize)));
			}
			if (tokens.size() > 1)
			{
				throw strus::runtime_error( _TXT("metadata feature tokenized to to more than one part"));
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
				ts = tokens.begin(), ti = tokens.begin(), te = tokens.end();
			for (; ti != te; ++ti)
			{
				m_searchTerms.push_back(
					analyzer::Term(
						feat.m_config->name(),
						feat.normalize( elem + ti->strpos, ti->strsize),
						rel_position + (samePosition?ts->docpos:ti->docpos)));
			}
			break;
		}
		case DocumentAnalyzer::FeatForwardIndexTerm:
		{
			std::vector<analyzer::Token>::const_iterator
				ts = tokens.begin(), ti = tokens.begin(), te = tokens.end();
			for (; ti != te; ++ti)
			{
				m_forwardTerms.push_back(
					analyzer::Term(
						feat.m_config->name(),
						feat.normalize( elem + ti->strpos, ti->strsize),
						rel_position + (samePosition?ts->docpos:ti->docpos)));
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
			res, ci->first, ci->second.position, ci->second.content.c_str(), ci->second.content.size(), false);
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

DocumentAnalyzerContext::DocumentAnalyzerContext( const DocumentAnalyzer* analyzer_, const DocumentClass& dclass, AnalyzerErrorBufferInterface* errorhnd)
	:m_analyzer(analyzer_)
	,m_segmenter(m_analyzer->m_segmenter->createContext( dclass))
	,m_parserContext(analyzer_->m_featurear)
	,m_eof(false)
	,m_last_position(0)
	,m_curr_position(0)
	,m_start_position(0)
	,m_errorhnd(errorhnd)
{
	if (!m_segmenter)
	{
		throw std::runtime_error( "failed to create document analyzer context");
	}
	m_subdocstack.push_back( analyzer::Document());
}

void DocumentAnalyzerContext::putInput( const char* chunk, std::size_t chunksize, bool eof)
{
	m_segmenter->putInput( chunk, chunksize, eof);
	m_eof = eof;
}

bool DocumentAnalyzerContext::analyzeNext( analyzer::Document& doc)
{
	try
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
							processDocumentSegment( doc, si->featidx, rel_position, si->elem.c_str(), si->elem.size(), true);
						}
						m_succChunks.clear();
	
						// process what is left to process for the current sub document:
						processConcatenated( doc);
						mapPositions( doc);
						mapStatistics( doc);
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
									processDocumentSegment( doc, si->featidx, rel_position, si->elem.c_str(), si->elem.size(), true);
								}
								m_succChunks.clear();
	
								// process this chunk:
								m_last_position = m_curr_position;
								processDocumentSegment( doc, featidx, rel_position, elem, elemsize, false);
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
								processDocumentSegment( doc, featidx, rel_position, elem, elemsize, true);
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
			processDocumentSegment( doc, si->featidx, rel_position, si->elem.c_str(), si->elem.size(), true);
		}
	
		// process concatenated chunks:
		processConcatenated( doc);
	
		// create real positions for output:
		mapPositions( doc);
		mapStatistics( doc);
		clearTermMaps();
	
		if (!doc.attributes().empty()) return true;
		if (!doc.metadata().empty()) return true;
		if (!doc.searchIndexTerms().empty()) return true;
		if (!doc.forwardIndexTerms().empty()) return true;
		return false;
	}
	CATCH_ERROR_MAP_RETURN( "error in DocumentAnalyzerContext::analyzeNext: %s", *m_errorhnd, false);
}

