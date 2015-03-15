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

void QueryAnalyzer::definePhraseType(
		const std::string& phraseType,
		const std::string& featureType,
		const TokenizerConfig& tokenizer,
		const NormalizerConfig& normalizer)
{
	const TokenizerInterface* tk = m_textProcessor->getTokenizer( tokenizer.name());
	const NormalizerInterface* nm = m_textProcessor->getNormalizer( normalizer.name());
	utils::SharedPtr<TokenizerInterface::Argument> tkarg( tk->createArgument( tokenizer.arguments()));
	utils::SharedPtr<NormalizerInterface::Argument> nmarg( nm->createArgument( m_textProcessor, normalizer.arguments()));

	m_featuremap[ utils::tolower( phraseType)]
		= FeatureConfig( featureType, tk, tkarg, nm, nmarg);
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
	utils::ScopedPtr<NormalizerInterface::Context> normctx( feat.normalizer()->createContext( feat.normalizerarg()));
	if (feat.tokenizerarg() && !tokctx.get())
	{
		throw std::runtime_error("internal: arguments defined for tokenizer but context constructor is empty");
	}
	if (feat.normalizerarg() && !normctx.get())
	{
		throw std::runtime_error("internal: arguments defined for tokenizer but context constructor is empty");
	}
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
		std::string val
			= feat.normalizer()->normalize(
				normctx.get(), content.c_str() + pi->strpos, pi->strsize);
		rt.push_back( analyzer::Term( feat.featureType(), val, posidx));
	}
	return rt;
}



