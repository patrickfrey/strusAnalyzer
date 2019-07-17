/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of a parametrizable document analyzer instance able to process many different document types
/// \file documentAnalyzerMap.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_ANALYZER_MAP_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_ANALYZER_MAP_IMPLEMENTATION_HPP_INCLUDED
#include "strus/documentAnalyzerMapInterface.hpp"
#include "strus/documentAnalyzerInstanceInterface.hpp"
#include "strus/documentAnalyzerContextInterface.hpp"
#include "strus/reference.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/documentAnalyzerMapView.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceInterface;
/// \brief Forward declaration
class AnalyzerObjectBuilderInterface;


/// \brief Defines a program for analyzing a document, splitting it into normalized terms that can be fed to the strus IR engine
class DocumentAnalyzerMap
	:public DocumentAnalyzerMapInterface
{
public:
	DocumentAnalyzerMap( const AnalyzerObjectBuilderInterface* objbuilder_, ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_objbuilder(objbuilder_),m_mimeTypeAnalyzerMap(),m_schemaAnalyzerMap(),m_analyzers(){}
	virtual ~DocumentAnalyzerMap(){}

	virtual DocumentAnalyzerInstanceInterface* createAnalyzer(
			const std::string& mimeType,
			const std::string& schema) const;

	virtual void addAnalyzer(
			const std::string& mimeType,
			const std::string& schema,
			DocumentAnalyzerInstanceInterface* analyzer_);

	virtual const DocumentAnalyzerInstanceInterface* getAnalyzer(
			const std::string& mimeType,
			const std::string& schema) const;

	virtual analyzer::Document analyze(
			const std::string& content,
			const analyzer::DocumentClass& dclass) const;

	virtual DocumentAnalyzerContextInterface* createContext(
			const analyzer::DocumentClass& dclass) const;

	virtual analyzer::DocumentAnalyzerMapView view() const;

private:
	typedef strus::Reference<DocumentAnalyzerInstanceInterface> DocumentAnalyzerReference;
	typedef std::map<std::string,const DocumentAnalyzerInstanceInterface*> Map;

private:
	ErrorBufferInterface* m_errorhnd;
	const AnalyzerObjectBuilderInterface* m_objbuilder;
	Map m_mimeTypeAnalyzerMap;
	Map m_schemaAnalyzerMap;
	std::vector<DocumentAnalyzerReference> m_analyzers;
};

}//namespace
#endif

