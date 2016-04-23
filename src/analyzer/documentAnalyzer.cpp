/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "documentAnalyzer.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/errorBufferInterface.hpp"
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
#include <limits>

#undef STRUS_LOWLEVEL_DEBUG

using namespace strus;

DocumentAnalyzer::DocumentAnalyzer( const SegmenterInterface* segmenter_, ErrorBufferInterface* errorhnd)
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
	:m_name(utils::tolower(name_))
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

static void freeNormalizers( const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	std::vector<NormalizerFunctionInstanceInterface*>::const_iterator
		ci = normalizers.begin(), ce = normalizers.end();
	for (; ci != ce; ++ci)
	{
		delete *ci;
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
	try
	{
		if (m_featurear.size()+1 >= MaxNofFeatures)
		{
			m_errorhnd->report( _TXT("number of features defined exceeds maximum limit"));
		}
		m_featurear.reserve( m_featurear.size()+1);
		m_featurear.push_back( FeatureConfig( name, tokenizer, normalizers, featureClass, options));
		m_segmenter->defineSelectorExpression( m_featurear.size(), expression);
	}
	catch (const std::bad_alloc&)
	{
		freeNormalizers( normalizers);
		delete tokenizer;
		m_errorhnd->report(_TXT("memory allocation error defining feature"));
	}
	catch (const std::runtime_error& err)
	{
		freeNormalizers( normalizers);
		delete tokenizer;
		m_errorhnd->report(_TXT("error defining feature: '%s'"), err.what());
	}
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
		// PF:NOTE: The following order of code ensures that if this constructor fails statfunc is not copied and can be freed by this function:
		m_statistics.reserve( m_statistics.size()+1);
		m_statistics.push_back( StatisticsConfig( fieldname, statfunc));
	}
	catch (const std::bad_alloc&)
	{
		delete statfunc;
		m_errorhnd->report(_TXT("memory allocation error defining aggregated meta data"));
	}
	catch (const std::runtime_error& err)
	{
		delete statfunc;
		m_errorhnd->report(_TXT("error defining aggregated meta data: '%s'"), err.what());
	}
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
		throw strus::runtime_error( _TXT("failed to create tokenizer context"));
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
			throw strus::runtime_error( _TXT("analyzed content incomplete or empty"));
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in DocumentAnalyzer::analyze: %s"), *m_errorhnd, analyzer::Document());
}

DocumentAnalyzerContextInterface* DocumentAnalyzer::createContext( const DocumentClass& dclass) const
{
	try
	{
		return new DocumentAnalyzerContext( this, dclass, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in DocumentAnalyzer::createContext: %s"), *m_errorhnd, 0);
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
		double value = si->statfunc()->evaluate( res);
#ifdef STRUS_LOWLEVEL_DEBUG
		std::cout << "add aggregated metadata " << si->name() << " " << value << std::endl;
		if (value * value < std::numeric_limits<double>::epsilon())
		{
			value = si->statfunc()->evaluate( res);
		}
#endif
		res.setMetaData( si->name(), value);
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
				std::string valuestr = feat.normalize( elem + tokens[0].strpos, tokens[0].strsize);
				NumericVariant value;
				if (!value.initFromString( valuestr.c_str()))
				{
					throw strus::runtime_error(_TXT("cannot convert nromalized item to number (metadata element): %s"), valuestr.c_str());
				}
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "add metadata " << feat.m_config->name() << "=" << valuestr << std::endl;
#endif
				res.setMetaData( feat.m_config->name(), value);
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
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "add attribute " << feat.m_config->name() << "=" << feat.normalize( elem + ti->strpos, ti->strsize) << std::endl;
#endif
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
				analyzer::Term term(
					feat.m_config->name(),
					feat.normalize( elem + ti->strpos, ti->strsize),
					rel_position + (samePosition?ts->docpos:ti->docpos));
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "add search index term " << "[" << term.pos() << "] " << term.type() << " " << term.value() << std::endl;
#endif
				m_searchTerms.push_back( term);
			}
			break;
		}
		case DocumentAnalyzer::FeatForwardIndexTerm:
		{
			std::vector<analyzer::Token>::const_iterator
				ts = tokens.begin(), ti = tokens.begin(), te = tokens.end();
			for (; ti != te; ++ti)
			{
				analyzer::Term term(
					feat.m_config->name(),
					feat.normalize( elem + ti->strpos, ti->strsize),
					rel_position + (samePosition?ts->docpos:ti->docpos));
#ifdef STRUS_LOWLEVEL_DEBUG
				std::cout << "add forward index term " << "[" << term.pos() << "] " << term.type() << " " << term.value() << std::endl;
#endif
				m_forwardTerms.push_back( term);
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

DocumentAnalyzerContext::DocumentAnalyzerContext( const DocumentAnalyzer* analyzer_, const DocumentClass& dclass, ErrorBufferInterface* errorhnd)
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
		throw strus::runtime_error( _TXT("failed to create document analyzer context"));
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
	AGAIN:
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
				std::string chunk( elem, elemsize);
				throw strus::runtime_error( _TXT( "error in analyze when processing chunk (%s): %s"), chunk.c_str(), err.what());
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
	
		// create output (with real positions):
		mapPositions( doc);

		// Map statistics, if defined
		bool rt = (doc.metadata().size() + doc.attributes().size() + doc.searchIndexTerms().size() + doc.forwardIndexTerms().size() != 0);
		if (rt)
		{
			mapStatistics( doc);
		}
		clearTermMaps();
		if (have_document && !rt)
		{
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "got empty document" << std::endl;
#endif
			goto AGAIN;
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in DocumentAnalyzerContext::analyzeNext: %s"), *m_errorhnd, false);
}

