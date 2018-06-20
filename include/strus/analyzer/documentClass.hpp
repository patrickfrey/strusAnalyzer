/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing the MIME type plus some attributes that could be relevant for analysis of a document
/// \file documentClass.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_CLASS_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_CLASS_HPP_INCLUDED
#include <vector>
#include <string>
#include <cstring>

/// \brief strus toplevel namespace
namespace strus {
namespace analyzer {

/// \brief Defines a description of the properties of an original document processed by the segmenter
class DocumentClass
{
public:
	/// \brief Default constructor
	DocumentClass(){}
	/// \brief Constructor
	explicit DocumentClass(
			const std::string& mimeType_)		:m_mimeType(mimeType_){}
	/// \brief Constructor
	DocumentClass(
			const std::string& mimeType_,
			const std::string& encoding_)		:m_mimeType(mimeType_),m_encoding(encoding_){}
	/// \brief Constructor
	DocumentClass(
			const std::string& mimeType_,
			const std::string& encoding_,
			const std::string& scheme_)		:m_mimeType(mimeType_),m_scheme(scheme_),m_encoding(encoding_){}
	/// \brief Copy constructor
#if __cplusplus >= 201103L
	DocumentClass( DocumentClass&& ) = default;
	DocumentClass( const DocumentClass& ) = default;
	DocumentClass& operator= ( DocumentClass&& ) = default;
	DocumentClass& operator= ( const DocumentClass& ) = default;
#else
	DocumentClass( const DocumentClass& o)			:m_mimeType(o.m_mimeType),m_scheme(o.m_scheme),m_encoding(o.m_encoding){}
#endif

	/// \brief Set the MIME type of the document class
	/// \param[in] the document MIME type string
	void setMimeType( const std::string& mimeType_)		{m_mimeType = mimeType_;}
	/// \brief Set the scheme identifier of the document class
	/// \param[in] the document scheme identifier
	void setScheme( const std::string& scheme_)		{m_scheme = scheme_;}
	/// \brief Set the character set encoding of the document class
	/// \param[in] the character set encoding string
	void setEncoding( const std::string& encoding_)		{m_encoding = encoding_;}

	/// \brief Get the MIME type of the document class
	/// \return the document MIME type string
	const std::string& mimeType() const			{return m_mimeType;}
	/// \brief Get the scheme identifier of the document class
	/// \return the document scheme identifier
	const std::string& scheme() const			{return m_scheme;}
	/// \brief Get the character set encoding of the document class
	/// \return the character set encoding string
	const std::string& encoding() const			{return m_encoding;}

	/// \brief Evaluate if this document class definition is defined
	/// \return true if defined
	bool defined() const					{return !m_mimeType.empty();}
	/// \brief Evaluate the level of definition of the document class
	/// \return level of definition
	/// \note this method is used to weight different oppinions of document class detection
	int level() const					{return m_mimeType.empty()?0:(1+!m_scheme.empty()+!m_encoding.empty());}

private:
	std::string m_mimeType;
	std::string m_scheme;
	std::string m_encoding;
};

}}//namespace
#endif

