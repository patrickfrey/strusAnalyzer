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
#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>
#include <stdexcept>
#include <set>
#include <map>

using namespace strus;

QueryAnalyzer::QueryAnalyzer(
		const TextProcessorInterface* textProcessor_)
	:m_textProcessor(textProcessor_){}


void QueryAnalyzer::defineMethod(
		const std::string& method,
		const std::string& featureType,
		const TokenizerConfig& tokenizer,
		const NormalizerConfig& normalizer)
{
	const TokenizerInterface* tk = m_textProcessor->getTokenizer( tokenizer.name());
	const NormalizerInterface* nm = m_textProcessor->getNormalizer( normalizer.name());
	boost::shared_ptr<TokenizerInterface::Argument> tkarg( tk->createArgument( tokenizer.arguments()));
	boost::shared_ptr<NormalizerInterface::Argument> nmarg( nm->createArgument( normalizer.arguments()));

	m_featuremap[ boost::algorithm::to_lower_copy( method)]
		= FeatureConfig( featureType, tk, tkarg, nm, nmarg);
}

const QueryAnalyzer::FeatureConfig& QueryAnalyzer::featureConfig( const std::string& type) const
{
	std::map<std::string,FeatureConfig>::const_iterator
		fi = m_featuremap.find( boost::algorithm::to_lower_copy( type));
	if (fi == m_featuremap.end())
	{
		throw std::runtime_error(std::string("query feature type '") + type + "' is not defined");
	}
	return fi->second;
}

std::vector<analyzer::Term> QueryAnalyzer::analyzeChunk(
		const std::string& method,
		const std::string& content) const
{
	std::vector<analyzer::Term> rt;
	const FeatureConfig& feat = featureConfig( method);
	boost::scoped_ptr<TokenizerInterface::Context> tokctx( feat.tokenizer()->createContext( feat.tokenizerarg()));
	boost::scoped_ptr<NormalizerInterface::Context> normctx( feat.normalizer()->createContext( feat.normalizerarg()));
	std::vector<tokenizer::Token>
		pos = feat.tokenizer()->tokenize( tokctx.get(), content.c_str(), content.size());
	std::vector<tokenizer::Token>::const_iterator
		pi = pos.begin(), pe = pos.end();

	if (pi == pe) return rt;
	unsigned int prevpos = pi->pos;
	for (unsigned int posidx=1; pi != pe; ++pi)
	{
		if (pi->pos > prevpos)
		{
			posidx += 1;
			prevpos = pi->pos;
		}
		std::string val
			= feat.normalizer()->normalize(
				normctx.get(), content.c_str() + pi->pos, pi->size);
		rt.push_back( analyzer::Term( feat.featureType(), val, posidx));
	}
	return rt;
}



