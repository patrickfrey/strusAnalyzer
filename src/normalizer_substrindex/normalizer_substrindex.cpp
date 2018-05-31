/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "normalizer_substrindex.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/base/utf8.hpp"
#include "strus/base/bitset.hpp"
#include "strus/base/string_format.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "private/tokenizeHelpers.hpp"
#include "strus/analyzer/functionView.hpp"
#include "compactNodeTrie.hpp"
#include <cstring>

using namespace strus;

#define NORMALIZER_NAME "substrindex"

class SubStringIndexNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	SubStringIndexNormalizerInstance( ErrorBufferInterface* errorhnd_, const std::vector<std::string>& args_)
		:m_errorhnd(errorhnd_),m_startset(),m_trie(),m_substar(),m_keyar(args_),m_maxlen(0)
	{
		std::vector<std::string>::const_iterator ai = m_keyar.begin(), ae = m_keyar.end();
		for (int aidx=0; ai != ae; ++ai,++aidx)
		{
			if (ai->empty())
			{
				continue;
			}
			if (ai->size() > SubStringIndexNormalizerFunction::MaxSubStringLength)
			{
				throw std::runtime_error(_TXT("length of substring out of range"));
			}
			if ((int)ai->size() > m_maxlen)
			{
				m_maxlen = ai->size();
			}
			conotrie::CompactNodeTrie::NodeData index = m_substar.size();
			m_substar.push_back( strus::string_format( "%d", aidx));
			if (!m_trie.set( ai->c_str(), index)) throw std::bad_alloc();

			char const* ci = ai->c_str();
			m_startset.set( (unsigned char)*ci, true);
			for (; *ci; ++ci)
			{
				m_chrset.set( (unsigned char)*ci, true);
			}
		}
	}

	virtual std::string normalize(
			const char* src,
			std::size_t srcsize) const
	{
		try
		{
			std::string rt;
			char buf[ SubStringIndexNormalizerFunction::MaxSubStringLength+1];
			char const* si = src;
			for (; *si; ++si)
			{
				if (m_startset.test( (unsigned char)*si))
				{
					char* bi = buf;
					char const* xi = si;
					int bidx = 0;
					conotrie::CompactNodeTrie::NodeData candidate_index = m_substar.size();
					for (; bidx < m_maxlen && m_chrset.test( (unsigned char)*xi); ++xi,++bi,++bidx)
					{
						bi[0] = *xi;
						bi[1] = '\0';
						conotrie::CompactNodeTrie::NodeData index;
						if (m_trie.get( buf, index))
						{
							si = xi;
							candidate_index = index;
						}
					}
					if (candidate_index < (conotrie::CompactNodeTrie::NodeData)m_substar.size())
					{
						rt.append( m_substar[ candidate_index]);
					}
					else
					{
						rt.push_back( *si);
					}
				}
				else
				{
					rt.push_back( *si);
				}
			}
			return rt;
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in \"%s\" normalizer: %s"), NORMALIZER_NAME, *m_errorhnd, std::string());
	}

	virtual analyzer::FunctionView view() const
	{
		try
		{
			std::vector<analyzer::FunctionView::NamedParameter> args;
			std::vector<std::string>::const_iterator ki = m_keyar.begin(), ke = m_keyar.end();
			for (; ki != ke; ++ki)
			{
				args.push_back( analyzer::FunctionView::NamedParameter( "substring", *ki));
			}
			return analyzer::FunctionView( NORMALIZER_NAME, args);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
	bitset<256> m_startset;
	bitset<256> m_chrset;
	conotrie::CompactNodeTrie m_trie;
	std::vector<std::string> m_substar;
	std::vector<std::string> m_keyar;
	int m_maxlen;
};


NormalizerFunctionInstanceInterface* SubStringIndexNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	try
	{
		return new SubStringIndexNormalizerInstance( m_errorhnd, args);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in create \"%s\" normalizer instance: %s"), NORMALIZER_NAME, *m_errorhnd, 0);
}

