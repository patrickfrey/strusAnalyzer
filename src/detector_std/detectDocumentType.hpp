/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Detect encoding and type of document
/// \file detectDocumentType.hpp
#ifndef _STRUS_ANALYZER_DETECT_DOCUMENT_TYPE_HPP_INCLUDED
#define _STRUS_ANALYZER_DETECT_DOCUMENT_TYPE_HPP_INCLUDED
#include <utility>


/// \brief strus toplevel namespace
namespace strus
{

struct DocumentType
{
	enum MimeType {MimeBinary,MimeXML,MimeJSON,MimeTSV,MimeTEXT};

	MimeType mimetypeid;
	const char* mimetype;
	const char* encoding;

	DocumentType()
		:mimetypeid(MimeBinary),mimetype(0),encoding(0){}
	DocumentType( MimeType mimetypeid_, const char* mimetype_, const char* encoding_)
		:mimetypeid(mimetypeid_),mimetype(mimetype_),encoding(encoding_){}
	DocumentType( const DocumentType& o)
		:mimetypeid(o.mimetypeid),mimetype(o.mimetype),encoding(o.encoding){}
};

DocumentType detectDocumentType( const char* src, std::size_t srcsize, bool eof);

}
#endif

