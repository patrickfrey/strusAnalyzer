/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Structure describing the library for determining the content type of items in the analysis of a content
/// \file contentStatisticsLibrary.cpp
#include "contentStatisticsLibrary.hpp"
#include "strus/analyzer/token.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/contentStatisticsElementView.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <algorithm>

/// \brief strus toplevel namespace
using namespace strus;

void ContentStatisticsLibrary::addVisibleAttribute( const std::string& name)
{
	try
	{
		strus::scoped_lock lock( m_mutex);
		if (std::find( m_attributes.begin(), m_attributes.end(), name) == m_attributes.end()) return;
		m_attributes.push_back( name);
	}
	CATCH_ERROR_MAP( _TXT("error in content statistics library: %s"), *m_errorhnd);
}

std::vector<std::string> ContentStatisticsLibrary::collectedAttributes() const
{
	try
	{
		strus::scoped_lock lock( m_mutex);
		return m_attributes;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in content statistics library: %s"), *m_errorhnd, std::vector<std::string>());
}

void ContentStatisticsLibrary::addElement(
		const std::string& type,
		const std::string& regexstr,
		int priority,
		int minLength,
		int maxLength,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)
{
	TokenizerFunctionReference tk;
	std::vector<NormalizerFunctionReference> na;
	try
	{
		strus::scoped_lock lock( m_mutex);

		if (priority < 0) throw std::runtime_error(_TXT("priority must be non-negative"));

		RegexSearchReference regex;
		if (!regexstr.empty() && regexstr != ".*")
		{
			regex.reset( new RegexSearch( regexstr, 0, m_errorhnd));
		}
		tk.reset( tokenizer);

		std::vector<NormalizerFunctionInstanceInterface*>::const_iterator ni = normalizers.begin(), ne = normalizers.end();
		for (; ni != ne; ++ni)
		{
			NormalizerFunctionReference nf( *ni);
			na.push_back( nf);
		}
		Element element( type, regexstr, regex, priority, minLength, maxLength, tk, na);
		m_ar.push_back( element);
		return;/*done*/
	}
	catch (const std::bad_alloc&)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
	}
	catch (const std::runtime_error& err)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("error building content statistics library: %s"), err.what());
	}
	// Cleanup garbagge:
	std::vector<NormalizerFunctionReference>::iterator li = na.begin(), le = na.end();
	for (; li != le; ++li) li->release();
	tk.release();
	if (tokenizer) delete tokenizer;
	std::vector<NormalizerFunctionInstanceInterface*>::const_iterator ni = normalizers.begin(), ne = normalizers.end();
	for (; ni != ne; ++ni) if (*ni) delete *ni;
}

std::vector<std::string> ContentStatisticsLibrary::matches( const char* input, std::size_t inputsize) const
{
	try
	{
		strus::scoped_lock lock( m_mutex);

		std::vector<std::string> rt;
		if (m_errorhnd->hasError()) return std::vector<std::string>();

		int priority = -1;
		std::vector<Element>::const_iterator ai = m_ar.begin(), ae = m_ar.end();
		for (; ai != ae; ++ai)
		{
			int len = ai->regex.get() ? ai->regex->find_start( input, input + inputsize) : inputsize;
			if (len == (int)inputsize)
			{
				std::vector<analyzer::Token> tokens = ai->tokenizer->tokenize( input, inputsize);
				if (ai->minLength >= 0 && (int)tokens.size() < ai->minLength) continue;
				if (ai->maxLength >= 0 && (int)tokens.size() > ai->maxLength) continue;
				if (ai->priority < priority) continue;

				std::vector<analyzer::Token>::const_iterator ti = tokens.begin(), te = tokens.end();
				for (; ti != te; ++ti)
				{
					std::string val( input + ti->origpos().ofs(), ti->origsize());
					std::vector<NormalizerFunctionReference>::const_iterator
						ni = ai->normalizers.begin(), ne = ai->normalizers.end();
					for (; ni != ne; ++ni)
					{
						val = (*ni)->normalize( val.c_str(), val.size());
						if (m_errorhnd->hasError()) break;
					}
				}
				if (m_errorhnd->hasError())
				{
					(void)m_errorhnd->fetchError();
				}
				else
				{
					if (ai->priority > priority)
					{
						rt.clear();
						priority = ai->priority;
					}
					rt.push_back( ai->type);
				}
			}
			if (m_errorhnd->hasError())
			{
				(void)m_errorhnd->fetchError();
			}
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in content statistics library: %s"), *m_errorhnd, std::vector<std::string>());
}

StructView ContentStatisticsLibrary::view() const
{
	try
	{
		strus::scoped_lock lock( m_mutex);

		StructView rt;
		std::vector<Element>::const_iterator ai = m_ar.begin(), ae = m_ar.end();
		for (; ai != ae; ++ai)
		{
			StructView normalizerviews;
			std::vector<NormalizerFunctionReference>::const_iterator
				ni = ai->normalizers.begin(), ne = ai->normalizers.end();
			for (; ni != ne; ++ni)
			{
				normalizerviews( (*ni)->view());
			}
			analyzer::ContentStatisticsElementView elem( ai->type, ai->regexstr, ai->priority, ai->minLength, ai->maxLength, ai->tokenizer->view(), normalizerviews);
			rt( elem);
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in content statistics library introspection: %s"), *m_errorhnd, StructView());
}


