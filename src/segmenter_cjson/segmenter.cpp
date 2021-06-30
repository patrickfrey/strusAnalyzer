/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "segmenter.hpp"
#include "segmenterContext.hpp"
#include "contentIterator.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/base/string_conv.hpp"
#include "private/textEncoder.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"

using namespace strus;

#define SEGMENTER_NAME "cjson"

void JsonSegmenterInstance::defineSelectorExpression( int id, const std::string& expression)
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


void JsonSegmenterInstance::defineSubSection( int startId, int endId, const std::string& expression)
{
	try
	{
		m_automaton.defineSubSection( startId, endId, expression);
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error defining subsection for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}


SegmenterContextInterface* JsonSegmenterInstance::createContext( const analyzer::DocumentClass& dclass) const
{
	try
	{
		strus::Reference<strus::utils::TextEncoderBase> decoder;
		if (!dclass.encoding().empty() && !strus::caseInsensitiveEquals( dclass.encoding(), "utf-8"))
		{
			decoder.reset( utils::createTextDecoder( dclass.encoding().c_str()));
		}
		return new JsonSegmenterContext( m_errorhnd, &m_automaton, decoder);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

SegmenterMarkupContextInterface* JsonSegmenterInstance::createMarkupContext( const analyzer::DocumentClass& dclass, const std::string& content) const
{
	m_errorhnd->report( ErrorCodeNotImplemented, _TXT("document markup not implemented for '%s' segmenter"), SEGMENTER_NAME);
	return 0;
}

StructView JsonSegmenterInstance::view() const
{
	try
	{
		return StructView()( "cjson");
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
}

SegmenterInstanceInterface* JsonSegmenter::createInstance( const analyzer::SegmenterOptions& opts) const
{
	try
	{
		if (!opts.items().empty()) throw strus::runtime_error(_TXT("no options defined for segmenter '%s'"), SEGMENTER_NAME);
		return new JsonSegmenterInstance( m_errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

StructView JsonSegmenter::view() const
{
	try
	{
		return StructView()
			("name", name())
			("mimetype", mimeType())
			("description", _TXT("Segmenter for JSON (application/json) based on the cjson library for parsing json and textwolf for the xpath automaton"))
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
}

ContentIteratorInterface* JsonSegmenter::createContentIterator(
		const char* content,
		std::size_t contentsize,
		const std::vector<std::string>& attributes,
		const std::vector<std::string>& expressions,
		const analyzer::DocumentClass& dclass,
		const analyzer::SegmenterOptions& opts) const
{
	try
	{
		if (!opts.items().empty()) throw strus::runtime_error(_TXT("no options defined for segmenter '%s'"), SEGMENTER_NAME);
		strus::Reference<strus::utils::TextEncoderBase> decoder;
		if (dclass.defined() && !strus::caseInsensitiveEquals( dclass.encoding(), "utf-8"))
		{
			decoder.reset( utils::createTextDecoder( dclass.encoding().c_str()));
		}
		return new ContentIterator( ContentIterator( content, contentsize, attributes, expressions, decoder, m_errorhnd));
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating content iterator of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}


