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
#include "strus/base/configParser.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "private/tokenizeHelpers.hpp"
#include "strus/analyzer/functionView.hpp"
#include "compactNodeTrie.hpp"
#include <cstring>

using namespace strus;

class SubStringMapNormalizerInstance
	:public NormalizerFunctionInstanceInterface
{
public:
	SubStringMapNormalizerInstance( ErrorBufferInterface* errorhnd_, const std::map<std::string,std::string>& args_, const char* normalizername_)
		:m_errorhnd(errorhnd_),m_startset(),m_trie(),m_substar(),m_maxlen(0),m_normalizername(normalizername_)
	{
		std::map<std::string,std::string>::const_iterator ai = args_.begin(), ae = args_.end();
		for (; ai != ae; ++ai)
		{
			if (ai->first.size() > SubStringIndexNormalizerFunction::MaxSubStringLength)
			{
				throw std::runtime_error(_TXT("length of substring out of range"));
			}
			if ((int)ai->first.size() > m_maxlen)
			{
				m_maxlen = ai->first.size();
			}
			conotrie::CompactNodeTrie::NodeData index = m_substar.size();
			m_substar.push_back( ai->second);
			if (!m_trie.set( ai->first.c_str(), index)) throw std::bad_alloc();

			char const* ci = ai->first.c_str();
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
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in \"%s\" normalizer: %s"), m_normalizername, *m_errorhnd, std::string());
	}

	virtual analyzer::FunctionView view() const
	{
		try
		{
			std::vector<analyzer::FunctionView::NamedParameter> args;
			conotrie::CompactNodeTrie::const_iterator ti = m_trie.begin(), te = m_trie.end();
			for (; ti != te; ++ti)
			{
				args.push_back( analyzer::FunctionView::NamedParameter(
							"map", strus::string_format("'%s'='%s'",
							ti.key().c_str(), m_substar[ ti.data()].c_str())));
			}
			return analyzer::FunctionView( m_normalizername, args);
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in introspection of '%s' normalizer: %s"), m_normalizername, *m_errorhnd, analyzer::FunctionView());
	}

private:
	ErrorBufferInterface* m_errorhnd;
	bitset<256> m_startset;
	bitset<256> m_chrset;
	conotrie::CompactNodeTrie m_trie;
	std::vector<std::string> m_substar;
	int m_maxlen;
	const char* m_normalizername;
};


NormalizerFunctionInstanceInterface* SubStringIndexNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	try
	{
		std::map<std::string,std::string> map;
		if (args.empty()) throw std::runtime_error(_TXT("too few arguments"));
		std::vector<std::string>::const_iterator ai = args.begin(), ae = args.end();
		for (int aidx=0; ai != ae; ++ai,++aidx)
		{
			if (ai->empty()) continue;
			map[ *ai] = strus::string_format( "%d", aidx);
		}
		return new SubStringMapNormalizerInstance( m_errorhnd, map, "substrindex");
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in create \"%s\" normalizer instance: %s"), "substrindex", *m_errorhnd, 0);
}

NormalizerFunctionInstanceInterface* SubStringMapNormalizerFunction::createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const
{
	try
	{
		if (args.size() > 1) throw std::runtime_error(_TXT("too many arguments"));
		if (args.empty()) throw std::runtime_error(_TXT("too few arguments"));
		std::vector<std::pair<std::string,std::string> > kvpairs = strus::getAssignmentListItems( args[0], m_errorhnd);
		if (kvpairs.empty() && m_errorhnd->hasError()) throw std::runtime_error(_TXT("failed to parse comma ',' separated key value assignments"));
		std::map<std::string,std::string> map;
		std::vector<std::pair<std::string,std::string> >::const_iterator ki = kvpairs.begin(), ke = kvpairs.end();
		for (; ki != ke; ++ki)
		{
			map[ ki->first] = ki->second;
		}
		return new SubStringMapNormalizerInstance( m_errorhnd, map, "substrmap");
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in create \"%s\" normalizer instance: %s"), "substrmap", *m_errorhnd, 0);
}

