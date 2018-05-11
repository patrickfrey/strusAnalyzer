/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "segmenter.hpp"
#include "segmenterContext.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/base/string_conv.hpp"
#include "private/textEncoder.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"

using namespace strus;

#define SEGMENTER_NAME "cjson"

void SegmenterInstance::defineSelectorExpression( int id, const std::string& expression)
{
	try
	{
		if (expression.empty())
		{
			m_automaton.defineSelectorExpression( id, "//()");
		}
		else
		{
			m_automaton.defineSelectorExpression( id, expression);
		}
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error defining expression for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}


void SegmenterInstance::defineSubSection( int startId, int endId, const std::string& expression)
{
	try
	{
		m_automaton.defineSubSection( startId, endId, expression);
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error defining subsection for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}


SegmenterContextInterface* SegmenterInstance::createContext( const analyzer::DocumentClass& dclass) const
{
	try
	{
		strus::Reference<strus::utils::TextEncoderBase> encoder;
		if (dclass.defined() && !strus::caseInsensitiveEquals( dclass.encoding(), "utf-8"))
		{
			encoder.reset( utils::createTextEncoder( dclass.encoding().c_str()));
		}
		return new SegmenterContext( m_errorhnd, &m_automaton, encoder);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

SegmenterMarkupContextInterface* SegmenterInstance::createMarkupContext( const analyzer::DocumentClass& dclass, const std::string& content) const
{
	m_errorhnd->report( ErrorCodeNotImplemented, _TXT("document markup not implemented for '%s' segmenter"), SEGMENTER_NAME);
	return 0;
}

analyzer::FunctionView SegmenterInstance::view() const
{
	try
	{
		return analyzer::FunctionView( "cjson")
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
}

SegmenterInstanceInterface* Segmenter::createInstance( const analyzer::SegmenterOptions& opts) const
{
	try
	{
		if (!opts.items().empty()) throw strus::runtime_error(_TXT("no options defined for segmenter '%s'"), SEGMENTER_NAME);
		return new SegmenterInstance( m_errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

const char* Segmenter::getDescription() const
{
	return _TXT("Segmenter for JSON (application/json) based on the cjson library for parsing json and textwolf for the xpath automaton");
}

