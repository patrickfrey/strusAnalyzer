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
#include "queryAnalyzer.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/normalizerInterface.hpp"
#include "strus/tokenizerInterface.hpp"
#include "private/utils.hpp"
#include <stdexcept>
#include <set>
#include <map>

using namespace strus;

QueryAnalyzer::QueryAnalyzer(
		const TextProcessorInterface* textProcessor_)
	:m_textProcessor(textProcessor_){}


QueryAnalyzer::FeatureConfig::FeatureConfig(
		const std::string& featureType_,
		const TextProcessorInterface* textProcessor_,
		const TokenizerConfig& tokenizerConfig,
		const std::vector<NormalizerConfig>& normalizerConfig)
	:m_featureType(featureType_)
	,m_tokenizer(0)
{
	m_tokenizer = textProcessor_->getTokenizer( tokenizerConfig.name());
	m_tokenizerarg.reset( m_tokenizer->createArgument( tokenizerConfig.arguments()));
	if (!m_tokenizerarg.get() && !tokenizerConfig.arguments().empty())
	{
		throw std::runtime_error( std::string( "no arguments expected for tokenizer '") + tokenizerConfig.name() + "'");
	}
	m_normalizerlist = NormalizerDef::getNormalizerDefList( textProcessor_, normalizerConfig);
}

QueryAnalyzer::FeatureContext::FeatureContext( const QueryAnalyzer::FeatureConfig& config)
	:m_config(&config)
	,m_tokenizerContext( config.tokenizer()->createContext( config.tokenizerarg()))
{
	std::vector<NormalizerDef>::const_iterator
		ni = config.normalizerlist().begin(),
		ne = config.normalizerlist().end();
	
	for (; ni != ne; ++ni)
	{
		m_normalizerContextAr.push_back( 
			ni->normalizer->createContext( ni->normalizerarg.get()));
	}
}

std::string QueryAnalyzer::FeatureContext::normalize( char const* tok, std::size_t toksize)
{
	std::vector<NormalizerDef>::const_iterator
		ni = m_config->normalizerlist().begin(),
		ne = m_config->normalizerlist().end();
	std::vector<utils::SharedPtr<NormalizerInterface::Context> >::iterator
		ci = m_normalizerContextAr.begin(),
		ce = m_normalizerContextAr.end();

	std::string rt;
	std::string origstr;
	for (; ni != ne && ci != ce; ++ni,++ci)
	{
		rt = ni->normalizer->normalize( ci->get(), tok, toksize);
		if (ni + 1 != ne)
		{
			origstr.swap( rt);
			tok = origstr.c_str();
			toksize = origstr.size();
		}
	}
	return rt;
}


void QueryAnalyzer::definePhraseType(
		const std::string& phraseType,
		const std::string& featureType,
		const TokenizerConfig& tokenizer,
		const std::vector<NormalizerConfig>& normalizer)
{
	m_featuremap[ utils::tolower( phraseType)]
		= FeatureConfig( featureType, m_textProcessor, tokenizer, normalizer);
}

const QueryAnalyzer::FeatureConfig& QueryAnalyzer::featureConfig( const std::string& phraseType) const
{
	std::map<std::string,FeatureConfig>::const_iterator
		fi = m_featuremap.find( utils::tolower( phraseType));
	if (fi == m_featuremap.end())
	{
		throw std::runtime_error(std::string( "query feature constructor for phrase type '") + phraseType + "' is not defined");
	}
	return fi->second;
}

std::vector<analyzer::Term>
	QueryAnalyzer::analyzePhrase(
		const std::string& phraseType,
		const std::string& content) const
{
	std::vector<analyzer::Term> rt;
	const FeatureConfig& feat = featureConfig( phraseType);
	utils::ScopedPtr<TokenizerInterface::Context> tokctx( feat.tokenizer()->createContext( feat.tokenizerarg()));
	FeatureContext ctx( feat);

	std::vector<analyzer::Token>
		pos = feat.tokenizer()->tokenize( tokctx.get(), content.c_str(), content.size());
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



