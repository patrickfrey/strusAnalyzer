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
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

/// \brief strus toplevel namespace
using namespace strus;

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
		if (priority < 0) throw std::runtime_error(_TXT("priority must be non-negative"));

		RegexSearchReference regex( new RegexSearch( regexstr, 0, m_errorhnd));
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
		std::vector<std::string> rt;
		if (m_errorhnd->hasError()) return std::vector<std::string>();

		int priority = -1;
		std::vector<Element>::const_iterator ai = m_ar.begin(), ae = m_ar.end();
		for (; ai != ae; ++ai)
		{
			int len = ai->regex->match_start( input, input + inputsize);
			if (len == (int)inputsize)
			{
				std::vector<analyzer::Token> tokens = ai->tokenizer->tokenize( input, inputsize);
				if (ai->minLength >= 0 && (int)tokens.size() < ai->minLength) continue;
				if (ai->maxLength >= 0 && (int)tokens.size() > ai->maxLength) continue;
				if (ai->priority < priority) continue;

				std::vector<analyzer::Token>::const_iterator ti = tokens.begin(), te = tokens.end();
				for (; ti != te; ++ti)
				{
					std::string val( input + ti->origpos(), ti->origsize());
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

std::vector<analyzer::ContentStatisticsElementView> ContentStatisticsLibrary::view() const
{
	try
	{
		std::vector<analyzer::ContentStatisticsElementView> rt;
		std::vector<Element>::const_iterator ai = m_ar.begin(), ae = m_ar.end();
		for (; ai != ae; ++ai)
		{
			std::vector<analyzer::FunctionView> normalizerviews;
			std::vector<NormalizerFunctionReference>::const_iterator
				ni = ai->normalizers.begin(), ne = ai->normalizers.end();
			for (; ni != ne; ++ni)
			{
				normalizerviews.push_back( (*ni)->view());
			}
			analyzer::ContentStatisticsElementView elem( ai->type, ai->regexstr, ai->tokenizer->view(), normalizerviews);
			rt.push_back( elem);
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in content statistics library introspection: %s"), *m_errorhnd, std::vector<analyzer::ContentStatisticsElementView>());
}


