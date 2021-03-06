/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the introspection for the document analyzer
/// \file documentAnalyzerIntrospection.hpp
#ifndef _STRUS_ANALYZER_INTROSPECTION_DOCUMENT_ANALYZER_HPP_INCLUDED
#define _STRUS_ANALYZER_INTROSPECTION_DOCUMENT_ANALYZER_HPP_INCLUDED
#include "strus/introspectionInterface.hpp"

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class DocumentAnalyzer;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \class DocumentAnalyzerIntrospection
/// \brief Implementation of the introspection for the document analyzer
class DocumentAnalyzerIntrospection
	:public IntrospectionInterface
{
public:
	DocumentAnalyzerIntrospection( const DocumentAnalyzer* analyzer, class ErrorBufferInterface* errorhnd);

	virtual ~DocumentAnalyzerIntrospection();
	virtual IntrospectionInterface* open( const std::string& name) const;
	virtual std::string value() const;
	virtual std::vector<std::string> list() const;

private:
	const DocumentAnalyzer* m_analyzer;
	class ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

