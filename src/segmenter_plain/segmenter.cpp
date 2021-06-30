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

#define SEGMENTER_NAME "plain"

void PlainSegmenterInstance::defineSelectorExpression( int id, const std::string& expression)
{
	try
	{
		if (!expression.empty())
		{
			m_errorhnd->report( ErrorCodeInvalidArgument, _TXT("only empty expressions allowed for '%s' segmenter, got '%s' for %d"), SEGMENTER_NAME, expression.c_str(), id);
		}
		m_segids.insert( id);
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error defining expression for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}


void PlainSegmenterInstance::defineSubSection( int startId, int endId, const std::string& expression)
{
	m_errorhnd->report( ErrorCodeNotImplemented, _TXT("not implemented: '%s'"), "PlainSegmenterInstance::defineSubSection");
}


SegmenterContextInterface* PlainSegmenterInstance::createContext( const analyzer::DocumentClass& dclass) const
{
	try
	{
		strus::Reference<strus::utils::TextEncoderBase> decoder;
		if (!dclass.encoding().empty() && !strus::caseInsensitiveEquals( dclass.encoding(), "utf-8"))
		{
			decoder.reset( strus::utils::createTextDecoder( dclass.encoding().c_str()));
		}
		return new PlainSegmenterContext( m_errorhnd, &m_segids, decoder);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

SegmenterMarkupContextInterface* PlainSegmenterInstance::createMarkupContext( const analyzer::DocumentClass& dclass, const std::string& content) const
{
	m_errorhnd->report( ErrorCodeNotImplemented, _TXT("document markup not implemented for '%s' segmenter"), SEGMENTER_NAME);
	return 0;
}

StructView PlainSegmenterInstance::view() const
{
	try
	{
		return StructView()( "name","plain")
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
}

SegmenterInstanceInterface* PlainSegmenter::createInstance( const analyzer::SegmenterOptions& opts) const
{
	try
	{
		if (!opts.items().empty()) throw strus::runtime_error(_TXT("no options defined for segmenter '%s'"), SEGMENTER_NAME);
		return new PlainSegmenterInstance( m_errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

StructView PlainSegmenter::view() const
{
	try
	{
		return StructView()
			("name", name())
			("mimetype", mimeType())
			("description", _TXT("Segmenter for plain text (in one segment)"))
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
}

ContentIterator::ContentIterator(
		const char* content_,
		std::size_t contentsize_,
		const std::vector<std::string>& attributes_,
		const std::vector<std::string>& expressions_,
		const strus::Reference<strus::utils::TextEncoderBase>& decoder_,
		ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_attributes(attributes_.begin(),attributes_.end()),m_content(),m_eof(false),m_decoder(decoder_)
{
	try
	{
		if (m_decoder.get())
		{
			m_content = m_decoder->convert( content_, contentsize_, true);
		}
		else
		{
			m_content = std::string( content_, contentsize_);
		}
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}

strus::ContentIteratorInterface* PlainSegmenter::createContentIterator(
		const char* content,
		std::size_t contentsize,
		const std::vector<std::string>& attributes,
		const std::vector<std::string>& expressions,
		const strus::analyzer::DocumentClass& dclass,
		const strus::analyzer::SegmenterOptions& opts) const
{
	try
	{
		strus::Reference<strus::utils::TextEncoderBase> decoder;
		if (dclass.defined() && !strus::caseInsensitiveEquals( dclass.encoding(), "utf-8"))
		{
			decoder.reset( strus::utils::createTextEncoder( dclass.encoding().c_str()));
		}
		return new ContentIterator( content, contentsize, attributes, expressions, decoder, m_errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, NULL);
}


