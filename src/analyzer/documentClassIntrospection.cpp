/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the introspection for the analyzer document class
/// \file documentClassIntrospection.cpp
#include "documentClassIntrospection.hpp"
#include "strus/base/introspection.hpp"
#include "documentAnalyzer.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include <new>

using namespace strus;

IntrospectionInterface* DocumentClassIntrospection::open( const std::string& name) const
{
	try
	{
		if (name == "mimetype")
		{
			return new AtomicTypeIntrospection<std::string>( &m_documentClass->mimeType(), m_errorhnd);
		}
		else if (name == "encoding")
		{
			return new AtomicTypeIntrospection<std::string>( &m_documentClass->encoding(), m_errorhnd);
		}
		else if (name == "scheme")
		{
			return new AtomicTypeIntrospection<std::string>( &m_documentClass->scheme(), m_errorhnd);
		}
		else
		{
			return NULL;
		}
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating introspection: %s"), *m_errorhnd, NULL);
}

std::string DocumentClassIntrospection::value() const
{
	return std::string();
}

std::vector<std::string> DocumentClassIntrospection::list() const
{
	static const char* ar[] = {"mimetype","encoding","scheme",0};
	return strus::getIntrospectionElementList( ar, m_errorhnd);
}

IntrospectionInterface* DocumentClassIntrospection::constructor( const void* self_, class ErrorBufferInterface* errorhnd)
{
	const analyzer::DocumentClass* self = (const analyzer::DocumentClass*)self_;
	try
	{
		return new DocumentClassIntrospection( self, errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error creating introspection: %s"), *errorhnd, NULL);
}

