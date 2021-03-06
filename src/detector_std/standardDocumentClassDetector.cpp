/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Standard document class detector implementation
/// \file standardDocumentClassDetector.hpp
#include "standardDocumentClassDetector.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "private/textEncoder.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/base/string_conv.hpp"
#include "strus/base/stdint.h"
#include "strus/base/local_ptr.hpp"
#include "detectDocumentType.hpp"
#include <cstring>
#include <stdexcept>

using namespace strus;

#define STRUS_DBGTRACE_COMPONENT_NAME "doctype"

StandardDocumentClassDetector::StandardDocumentClassDetector( const TextProcessorInterface* textproc_, ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_debugtrace(0),m_textproc(textproc_)
	,m_schemas(),m_xmlSegmenter(),m_jsonSegmenter(),m_tsvSegmenter()
{
	DebugTraceInterface* dbgi = m_errorhnd->debugTrace();
	if (dbgi) m_debugtrace = dbgi->createTraceContext( STRUS_DBGTRACE_COMPONENT_NAME);
}

static bool caseInsensitiveStringSearch( char const* src, const char* needle)
{
	char const* si = src;
	for (; *si; ++si)
	{
		if ((*si|32) == (*needle|32))
		{
			char const* ni = needle;
			for (++si,++ni; *ni && ((*si|32) == (*ni|32)); ++si,++ni){}
			if (!*ni) return true;
		}
	}
	return false;
}

static bool isMimeXml( const std::string& mimeType)
{
	return caseInsensitiveStringSearch( mimeType.c_str(), "xml");
}

static bool isMimeJson( const std::string& mimeType)
{
	return caseInsensitiveStringSearch( mimeType.c_str(), "json");
}

static bool isMimeTsv( const std::string& mimeType)
{
	return caseInsensitiveStringSearch( mimeType.c_str(), "tab");
}

SegmenterInstanceInterface* StandardDocumentClassDetector::getSegmenterInstance( const std::string& mimeType)
{
	if (isMimeXml( mimeType))
	{
		if (!m_xmlSegmenter.get())
		{
			const SegmenterInterface* sg = m_textproc->getSegmenterByMimeType( mimeType);
			if (!sg) return NULL;
			m_xmlSegmenter.reset( sg->createInstance());
		}
		return m_xmlSegmenter.get();
	}
	else if (isMimeJson( mimeType))
	{
		if (!m_jsonSegmenter.get())
		{
			const SegmenterInterface* sg = m_textproc->getSegmenterByMimeType( mimeType);
			if (!sg) return NULL;
			m_jsonSegmenter.reset( sg->createInstance());
		}
		return m_jsonSegmenter.get();
	}
	else if (isMimeTsv( mimeType))
	{
		if (!m_tsvSegmenter.get())
		{
			const SegmenterInterface* sg = m_textproc->getSegmenterByMimeType( mimeType);
			if (!sg) return NULL;
			m_tsvSegmenter.reset( sg->createInstance());
		}
		return m_tsvSegmenter.get();
	}
	return NULL;
}

enum {	MaxExpressions = 31, MaxSchemas = 256 };

struct Event
{
	bool select;
	int schemaidx;
	int expridx;

	Event( bool select_, int schemaidx_, int expridx_)
		:select(select_),schemaidx(schemaidx_),expridx(expridx_){}
#if __cplusplus >= 201103L
	Event( Event&& ) = default;
	Event( const Event& ) = default;
	Event& operator= ( Event&& ) = default;
	Event& operator= ( const Event& ) = default;
#else
	Event( const Event& o)
		:select(o.select),schemaidx(o.schemaidx),expridx(o.expridx){}
#endif
};

static inline Event getEvent( unsigned int evid)
{
	return Event( (evid >> 24), (evid & 0x00FFffFF) >> 16, evid & 0xffFF);
}
static unsigned int getEventId( bool select, unsigned int schemaidx, unsigned int expridx)
{
	return select ? ((schemaidx << 16) + expridx) : ((1 << 24) + (schemaidx << 16) + expridx);
}


void StandardDocumentClassDetector::defineDocumentSchemaDetector(
		const std::string& schema,
		const std::string& mimeType,
		const std::vector<std::string>& select_expressions,
		const std::vector<std::string>& reject_expressions)
{
	try
	{
		if ((std::size_t)MaxSchemas <= m_schemas.size()) throw std::runtime_error(_TXT("to many schemas defined"));
		if ((std::size_t)MaxExpressions < select_expressions.size()) throw std::runtime_error(_TXT("to many expressions defined"));
		if ((std::size_t)MaxExpressions < reject_expressions.size()) throw std::runtime_error(_TXT("to many expressions defined"));

		m_schemas.push_back( SchemaDef( schema, select_expressions, reject_expressions));
		unsigned int schemaIndex = m_schemas.size();

		SegmenterInstanceInterface* segmenter = getSegmenterInstance( mimeType);
		if (!segmenter)
		{
			throw strus::runtime_error(_TXT("cannot define schema detector for mime type '%s'"), mimeType.c_str());
		}
		std::vector<std::string>::const_iterator si = select_expressions.begin(), se = select_expressions.end();
		for (int sidx=1; si != se; ++si,++sidx)
		{
			unsigned int ev = getEventId( true, schemaIndex, sidx);
			segmenter->defineSelectorExpression( ev, *si);
		}
		std::vector<std::string>::const_iterator ri = reject_expressions.begin(), re = reject_expressions.end();
		for (int ridx=1; ri != re; ++ri,++ridx)
		{
			unsigned int ev = getEventId( false/*select*/, schemaIndex, ridx);
			segmenter->defineSelectorExpression( ev, *ri);
		}
	}
	CATCH_ERROR_MAP( _TXT("error in standard document class detector: %s"), *m_errorhnd);
}

int StandardDocumentClassDetector::detectSchema( const SegmenterInstanceInterface* segmenter, analyzer::DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize, bool isComplete) const
{
	enum {RejectFlag = 0xFFffFFff};
	uint32_t select_flags[ MaxSchemas];
	std::size_t sidx=0;
	for (; sidx < m_schemas.size(); ++sidx)
	{
		select_flags[ sidx] = ((1 << m_schemas[ sidx].nofEvents()) - 1) << 1;
		// ... bit at position 0 is for the reject event that can never be masked out
	}
	strus::local_ptr<SegmenterContextInterface> ctx( segmenter->createContext( dclass));

	if (!ctx.get()) throw strus::runtime_error(_TXT("unable to create segmenter for document type detection"));
	ctx->putInput( contentBegin, contentBeginSize, isComplete);
	int evid = 0;
	SegmenterPosition pos;
	const char* segment;
	std::size_t segmentsize;
	while (ctx->getNext( evid, pos, segment, segmentsize))
	{
		Event ev = getEvent( evid);
		if (ev.select)
		{
			select_flags[ ev.schemaidx-1] &= ~(1 << ev.expridx);
			if (m_debugtrace) m_debugtrace->event( "match", "schema %s expression %s set %x", m_schemas[ ev.schemaidx-1].name.c_str(), m_schemas[ ev.schemaidx-1].select_expressions[ ev.expridx-1].c_str(), (unsigned int)select_flags[ ev.schemaidx-1]);
		}
		else
		{
			select_flags[ ev.schemaidx-1] |= 1;
			if (m_debugtrace) m_debugtrace->event( "reject", "schema %s expression %s set %x", m_schemas[ ev.schemaidx-1].name.c_str(), m_schemas[ ev.schemaidx-1].reject_expressions[ ev.expridx-1].c_str(), (unsigned int)select_flags[ ev.schemaidx-1]);
		}
	}
	for (sidx=0; sidx < m_schemas.size(); ++sidx)
	{
		if (select_flags[ sidx] == 0)
		{
			if (m_debugtrace) m_debugtrace->event( "decide", "schema %s", m_schemas[ sidx].name.c_str());
			dclass.setSchema( m_schemas[ sidx].name);
			return true;
		}
	}
	if (m_debugtrace) m_debugtrace->event( "decide", "schema %s", "<unknown>");
	return false;
}

bool StandardDocumentClassDetector::detect( analyzer::DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize, bool isComplete) const
{
	try
	{
		if (m_debugtrace) m_debugtrace->open( "documentclass");
		DocumentType doctype = strus::detectDocumentType( contentBegin, contentBeginSize, isComplete);
		if (!doctype.mimetype)
		{
			std::runtime_error(_TXT("MIME type detection failed"));
		}
		else
		{
			if (m_debugtrace) m_debugtrace->event( "mime-type", "%s", doctype.mimetype);
			dclass = analyzer::DocumentClass( doctype.mimetype, doctype.encoding?doctype.encoding:"utf-8");
			switch (doctype.mimetypeid)
			{
				case DocumentType::MimeBinary:
					break;
				case DocumentType::MimeXML:
					if (m_xmlSegmenter.get())
					{
						(void)detectSchema( m_xmlSegmenter.get(), dclass, contentBegin, contentBeginSize, isComplete);
					}
					break;
				case DocumentType::MimeJSON:
					if (m_jsonSegmenter.get())
					{
						(void)detectSchema( m_jsonSegmenter.get(), dclass, contentBegin, contentBeginSize, isComplete);
					}
					break;
				case DocumentType::MimeTSV:
					if (m_tsvSegmenter.get())
					{
						(void)detectSchema( m_tsvSegmenter.get(), dclass, contentBegin, contentBeginSize, isComplete);
					}
					break;
				case DocumentType::MimeTEXT:
					break;
			}
			if (m_debugtrace) m_debugtrace->close();
			return true;
		}
		if (m_debugtrace) m_debugtrace->close();
		return false;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in standard document class detector: %s"), *m_errorhnd, false);
}

StructView StandardDocumentClassDetector::SchemaDef::view() const
{
	return StructView()
		("name", name)
		("select",select_expressions)
		("reject",reject_expressions);
}

StructView StandardDocumentClassDetector::view() const
{
	try
	{
		StructView schemaview;
		std::vector<SchemaDef>::const_iterator si = m_schemas.begin(), se = m_schemas.end();
		for (; si != se; ++si)
		{
			schemaview( si->view());
		}
		StructView segmenterview;
		segmenterview
			("xml", m_xmlSegmenter->name())
			("json", m_jsonSegmenter->name())
			("tsv", m_tsvSegmenter->name());
		return StructView()
			("name", "detector_std")
			("schemas", schemaview)
			("segmenter", segmenterview)
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
}


