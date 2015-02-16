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
#include "strus/tokenizer/token.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/scoped_ptr.hpp>
#include <stdexcept>
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

void DocumentAnalyzer::defineFeature(
	FeatureClass featureClass,
	const std::string& name,
	const std::string& expression,
	const TokenizerConfig& tokenizer,
	const NormalizerConfig& normalizer)
{
	const TokenizerInterface* tk = m_textProcessor->getTokenizer( tokenizer.name());
	const NormalizerInterface* nm = m_textProcessor->getNormalizer( normalizer.name());
	boost::shared_ptr<TokenizerInterface::Argument> tkarg( tk->createArgument( tokenizer.arguments()));
	if (!tkarg.get() && !tokenizer.arguments().empty())
	{
		throw std::runtime_error( std::string( "no arguments expected for tokenizer '") + tokenizer.name() + "'");
	}
	boost::shared_ptr<NormalizerInterface::Argument> nmarg( nm->createArgument( normalizer.arguments()));
	if (!nmarg.get() && !normalizer.arguments().empty())
	{
		throw std::runtime_error( std::string( "no arguments expected for normalizer '") + normalizer.name() + "'");
	}

	m_featurear.push_back( FeatureConfig( name, tk, tkarg, nm, nmarg, featureClass));
	m_segmenter->defineSelectorExpression( m_featurear.size(), expression);
}

class ParserContext
{
public:
	ParserContext( const std::vector<DocumentAnalyzer::FeatureConfig>& config)
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

	void cleanup()
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

	~ParserContext()
	{
		cleanup();
	}

	NormalizerInterface::Context* normalizerContext( int featidx)
	{
		if (featidx <= 0 || (std::size_t)featidx > m_size)
		{
			throw std::runtime_error( "internal: unknown index of feature");
		}
		return m_normalizerContextAr[ featidx-1];
	}

	TokenizerInterface::Context* tokenizerContext( int featidx)
	{
		if (featidx <= 0 || (std::size_t)featidx > m_size)
		{
			throw std::runtime_error( "internal: unknown index of feature");
		}
		return m_tokenizerContextAr[ featidx-1];
	}

private:
	TokenizerInterface::Context** m_tokenizerContextAr;
	NormalizerInterface::Context** m_normalizerContextAr;
	std::size_t m_size;
};


/// \brief Map byte offset positions to token occurrence positions:
static void mapPositions(
		analyzer::Document& res,
		const std::vector<analyzer::Term>& searchTerms_content,
		const std::vector<analyzer::Term>& searchTerms_pred,
		const std::vector<analyzer::Term>& searchTerms_succ,
		const std::vector<analyzer::Term>& forwardTerms_content,
		const std::vector<analyzer::Term>& forwardTerms_pred,
		const std::vector<analyzer::Term>& forwardTerms_succ)
{
	std::set<unsigned int> pset;
	std::vector<analyzer::Term>::const_iterator ri = searchTerms_content.begin(), re = searchTerms_content.end();
	for (; ri != re; ++ri)
	{
		pset.insert( ri->pos());
	}
	ri = forwardTerms_content.begin(), re = forwardTerms_content.end();
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
	for (ri = searchTerms_content.begin(), re = searchTerms_content.end(); ri != re; ++ri)
	{
		unsigned int pos = posmap[ ri->pos()];
		res.addSearchIndexTerm( ri->type(), ri->value(), pos);
	}
	for (ri = forwardTerms_content.begin(), re = forwardTerms_content.end(); ri != re; ++ri)
	{
		unsigned int pos = posmap[ ri->pos()];
		res.addForwardIndexTerm( ri->type(), ri->value(), pos);
	}
	for (ri = searchTerms_pred.begin(), re = searchTerms_pred.end(); ri != re; ++ri)
	{
		unsigned int pos = 1;
		if (!posmap.empty())
		{
			std::map<unsigned int, unsigned int>::const_iterator
				si = posmap.upper_bound( ri->pos());
			--si;
			pos = si->second;
		}
		res.addSearchIndexTerm( ri->type(), ri->value(), pos);
	}
	for (ri = forwardTerms_pred.begin(), re = forwardTerms_pred.end(); ri != re; ++ri)
	{
		unsigned int pos = 1;
		if (!posmap.empty())
		{
			std::map<unsigned int, unsigned int>::const_iterator
				si = posmap.upper_bound( ri->pos());
			--si;
			pos = si->second;
		}
		res.addForwardIndexTerm( ri->type(), ri->value(), pos);
	}
	for (ri = searchTerms_succ.begin(), re = searchTerms_succ.end(); ri != re; ++ri)
	{
		unsigned int pos = 1;
		if (!posmap.empty())
		{
			std::map<unsigned int, unsigned int>::const_iterator
				si = posmap.upper_bound( ri->pos());
			if (si == posmap.end())
			{
				pos = pcnt+1;
			}
			else
			{
				pos = si->second;
			}
		}
		res.addSearchIndexTerm( ri->type(), ri->value(), pos);
	}
	for (ri = forwardTerms_succ.begin(), re = forwardTerms_succ.end(); ri != re; ++ri)
	{
		unsigned int pos = 1;
		if (!posmap.empty())
		{
			std::map<unsigned int, unsigned int>::const_iterator
				si = posmap.upper_bound( ri->pos());
			if (si == posmap.end())
			{
				pos = pcnt+1;
			}
			else
			{
				pos = si->second;
			}
		}
		res.addForwardIndexTerm( ri->type(), ri->value(), pos);
	}
}


static void normalize(
		analyzer::Document& res,
		NormalizerInterface::Context* normctx,
		const DocumentAnalyzer::FeatureConfig& feat,
		const char* elem,
		std::size_t elemsize,
		const std::vector<tokenizer::Token>& pos,
		std::size_t curr_position,
		std::vector<analyzer::Term>& searchTerms,
		std::vector<analyzer::Term>& forwardTerms)
{

	switch (feat.featureClass())
	{
		case DocumentAnalyzer::FeatMetaData:
		{
			if (!pos.empty())
			{
				std::string valstr(
					feat.normalizer()->normalize(
						normctx, elem + pos[0].pos, pos[0].size));
				res.setMetaData( feat.name(), valstr);
			}
			if (pos.size() > 1)
			{
				throw std::runtime_error( "metadata feature tokenized to to more than one part");
			}
			break;
		}
		case DocumentAnalyzer::FeatAttribute:
		{
			std::vector<tokenizer::Token>::const_iterator
				pi = pos.begin(), pe = pos.end();
			for (; pi != pe; ++pi)
			{
				std::string valstr(
					feat.normalizer()->normalize(
						normctx, elem + pi->pos, pi->size));
				res.setAttribute( feat.name(), valstr);
			}
			break;
		}
		case DocumentAnalyzer::FeatSearchIndexTerm:
		{
			std::vector<tokenizer::Token>::const_iterator
				pi = pos.begin(), pe = pos.end();
			for (; pi != pe; ++pi)
			{
				std::string valstr(
					feat.normalizer()->normalize(
						normctx, elem + pi->pos, pi->size));
				searchTerms.push_back(
					analyzer::Term( feat.name(), valstr, curr_position + pi->pos));
			}
			break;
		}
		case DocumentAnalyzer::FeatForwardIndexTerm:
		{
			std::vector<tokenizer::Token>::const_iterator
				pi = pos.begin(), pe = pos.end();
			for (; pi != pe; ++pi)
			{
				std::string valstr(
					feat.normalizer()->normalize(
						normctx, elem + pi->pos, pi->size));
				forwardTerms.push_back(
					analyzer::Term( feat.name(), valstr, curr_position + pi->pos));
			}
			break;
		}
	}
}


struct Chunk
{
	Chunk()
		:selector(SegmenterInterface::Content),position(0){}
	Chunk( SegmenterInterface::SelectorType selector_, std::size_t position_, const std::string& content_)
		:selector(selector_),position(position_),content(content_){}
	Chunk( const Chunk& o)
		:selector(o.selector),position(o.position),content(o.content){}

	SegmenterInterface::SelectorType selector;
	std::size_t position;
	std::string content;
};

struct Segment
{
	unsigned int pos;
	unsigned int size;

	Segment( unsigned int pos_, unsigned int size_)
		:pos(pos_),size(size_){}
	Segment( const Segment& o)
		:pos(o.pos),size(o.size){}
};

analyzer::Document DocumentAnalyzer::analyze( const std::string& content) const
{
	analyzer::Document rt;
	boost::scoped_ptr<SegmenterInstanceInterface>
		segmenter( m_segmenter->createInstance( content));
	const char* elem = 0;
	std::size_t elemsize = 0;
	int featidx = 0;
	std::size_t curr_position = 0;
	typedef std::map<int,Chunk> ConcatenatedMap;
	ConcatenatedMap concatenatedMap;
	typedef std::map<int,Segment> AnnotationMap;
	AnnotationMap annotationMap;
	ParserContext ctx( m_featurear);
	std::vector<int> selectortypes;

	selectortypes.resize( m_featurear.size(), -1);
	std::vector<analyzer::Term> searchTerms_pred;
	std::vector<analyzer::Term> searchTerms_succ;
	std::vector<analyzer::Term> searchTerms_content;
	std::vector<analyzer::Term> forwardTerms_pred;
	std::vector<analyzer::Term> forwardTerms_succ;
	std::vector<analyzer::Term> forwardTerms_content;
	std::vector<analyzer::Term>* searchTerms = 0;
	std::vector<analyzer::Term>* forwardTerms = 0;

	// [1] Scan the document and push the normalized tokenization of the elements to the result:
	while (segmenter->getNext( featidx, curr_position, elem, elemsize))
	{
		try
		{
			int stype = selectortypes[ featidx-1];
			SegmenterInterface::SelectorType selectorType;
			if (stype < 0)
			{
				selectorType = m_segmenter->getSelectorType( featidx);
				selectortypes[ featidx-1] = (int)selectorType;
			}
			else
			{
				selectorType = (SegmenterInterface::SelectorType)stype;
			}
			switch (selectorType)
			{
				case SegmenterInterface::Content:
					searchTerms = &searchTerms_content;
					forwardTerms = &forwardTerms_content;
					break;
				case SegmenterInterface::AnnotationSuccessor:
					searchTerms = &searchTerms_succ;
					forwardTerms = &forwardTerms_succ;
					break;
				case SegmenterInterface::AnnotationPredecessor:
					searchTerms = &searchTerms_pred;
					forwardTerms = &forwardTerms_pred;
					break;
			}

			const DocumentAnalyzer::FeatureConfig& feat = featureConfig( featidx);
			TokenizerInterface::Context* tokctx = ctx.tokenizerContext( featidx);
			NormalizerInterface::Context* normctx = ctx.normalizerContext( featidx);

			if (feat.tokenizer()->concatBeforeTokenize())
			{
				ConcatenatedMap::iterator ci = concatenatedMap.find( featidx);
				if (ci == concatenatedMap.end())
				{
					concatenatedMap[ featidx]
						= Chunk( selectorType, curr_position,
							 std::string( elem, elemsize));
				}
				else
				{
					Chunk& cm = concatenatedMap[ featidx];
					std::size_t newlen = curr_position - cm.position;
					cm.content.resize( newlen, ' ');
					cm.content.append( elem, elemsize);
				}
				continue;
			}
			else
			{
				std::vector<tokenizer::Token>
					pos = feat.tokenizer()->tokenize( tokctx, elem, elemsize);

				normalize( rt, normctx, feat, elem, elemsize, pos, curr_position, *searchTerms, *forwardTerms);
			}
		}
		catch (const std::runtime_error& err)
		{
			throw std::runtime_error( std::string( "error in analyze when processing chunk (") + std::string( elem, elemsize) + "): " + err.what());
		}
	}
	ConcatenatedMap::const_iterator
		ci = concatenatedMap.begin(),
		ce = concatenatedMap.end();

	for (; ci != ce; ++ci)
	{
		const DocumentAnalyzer::FeatureConfig& feat = featureConfig( ci->first);
		TokenizerInterface::Context* tokctx = ctx.tokenizerContext( featidx);
		NormalizerInterface::Context* normctx = ctx.normalizerContext( featidx);

		std::vector<tokenizer::Token>
			pos = feat.tokenizer()->tokenize(
				tokctx,
				ci->second.content.c_str(),
				ci->second.content.size());

		normalize( rt, normctx, feat, ci->second.content.c_str(),
				ci->second.content.size(), pos, ci->second.position,
				*searchTerms, *forwardTerms);
	}
	mapPositions( rt, searchTerms_content, searchTerms_pred, searchTerms_succ, forwardTerms_content, forwardTerms_pred, forwardTerms_succ);
	return rt;
}


