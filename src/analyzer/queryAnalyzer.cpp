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
#include "queryAnalyzer.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/analyzerErrorBufferInterface.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <stdexcept>
#include <set>
#include <map>

using namespace strus;

QueryAnalyzer::FeatureConfig::FeatureConfig(
		const std::string& featureType_,
		TokenizerFunctionInstanceInterface* tokenizer_,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers_)
	:m_featureType(featureType_)
	,m_tokenizer(tokenizer_)
{
	// PF:NOTE: The following order of code ensures that if this constructor fails then no tokenizer or normalizer is copied, because otherwise they will be free()d twice:
	m_normalizerlist.reserve( normalizers_.size());
	std::vector<NormalizerFunctionInstanceInterface*>::const_iterator
		ci = normalizers_.begin(), ce = normalizers_.end();
	for (; ci != ce; ++ci)
	{
		m_normalizerlist.push_back( *ci);
	}
}

QueryAnalyzer::FeatureContext::FeatureContext( const QueryAnalyzer::FeatureConfig& config)
	:m_config(&config)
	,m_tokenizerContext( config.tokenizer()->createFunctionContext())
{
	if (!m_tokenizerContext.get())
	{
		throw strus::runtime_error( _TXT("failed to create tokenizer context"));
	}
	std::vector<FeatureConfig::NormalizerReference>::const_iterator
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

std::vector<analyzer::Token> QueryAnalyzer::FeatureContext::tokenize( char const* segment, std::size_t segmentsize)
{
	return m_tokenizerContext->tokenize( segment, segmentsize);
}

std::string QueryAnalyzer::FeatureContext::normalize( char const* tok, std::size_t toksize)
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


static void freeNormalizers( const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	std::vector<NormalizerFunctionInstanceInterface*>::const_iterator
		ci = normalizers.begin(), ce = normalizers.end();
	for (; ci != ce; ++ci)
	{
		delete *ci;
	}
}

void QueryAnalyzer::definePhraseType(
		const std::string& phraseType,
		const std::string& featureType,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	try
	{
		m_featuremap[ utils::tolower( phraseType)]
			= FeatureConfig( featureType, tokenizer, normalizers);
	}
	catch (const std::bad_alloc&)
	{
		freeNormalizers( normalizers);
		delete tokenizer;
		m_errorhnd->report(_TXT("memory allocation error defining phrase type"));
	}
	catch (const std::runtime_error& err)
	{
		freeNormalizers( normalizers);
		delete tokenizer;
		m_errorhnd->report(_TXT("error defining phrase type: '%s'"), err.what());
	}
}

const QueryAnalyzer::FeatureConfig& QueryAnalyzer::featureConfig( const std::string& phraseType) const
{
	std::map<std::string,FeatureConfig>::const_iterator
		fi = m_featuremap.find( utils::tolower( phraseType));
	if (fi == m_featuremap.end())
	{
		throw strus::runtime_error(_TXT( "query feature constructor for phrase type '%s' is not defined"), phraseType.c_str());
	}
	return fi->second;
}

std::vector<analyzer::Term>
	QueryAnalyzer::analyzePhrase(
		const std::string& phraseType,
		const std::string& content) const
{
	try
	{
		std::vector<analyzer::Term> rt;
		const FeatureConfig& feat = featureConfig( phraseType);
		FeatureContext ctx( feat);
	
		std::vector<analyzer::Token>
			pos = ctx.tokenize( content.c_str(), content.size());
		std::vector<analyzer::Token>::const_iterator
			pi = pos.begin(), pe = pos.end();
	
		if (pi == pe) return rt;
		unsigned int prevpos = pi->docpos;
		for (unsigned int posidx=1; pi != pe; ++pi)
		{
			if (pi->docpos > prevpos)
			{
				posidx += 1;
				prevpos = pi->docpos;
			}
			std::string val = ctx.normalize( content.c_str() + pi->strpos, pi->strsize);
			rt.push_back( analyzer::Term( feat.featureType(), val, posidx));
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in 'QueryAnalyzer::analyzePhrase': %s"), *m_errorhnd, std::vector<analyzer::Term>());
}

std::vector<analyzer::TermVector> QueryAnalyzer::analyzePhraseBulk(
		const std::vector<Phrase>& phraseBulk) const
{
	try
	{
		std::vector<analyzer::TermVector> rt;
		std::vector<Phrase>::const_iterator pi = phraseBulk.begin(), pe = phraseBulk.end();
		for (; pi != pe; ++pi)
		{
			rt.push_back( analyzePhrase( pi->type(), pi->content()));
		}
		return rt;
	}
	catch (const std::bad_alloc& err)
	{
		m_errorhnd->report( _TXT("memory alloc error in '%s'"), "QueryAnalyzer::analyzePhraseBulk");
		return std::vector<analyzer::TermVector>();
	}
}


