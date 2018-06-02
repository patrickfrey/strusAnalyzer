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
#include "strus/documentAnalyzerInterface.hpp"
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


/// \brief Defines a program for analyzing a document, splitting it into normalized terms that can be fed to the strus IR engine
class DocumentAnalyzerMap
	:public DocumentAnalyzerMapInterface
{
public:
	explicit DocumentAnalyzerMap( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}
	virtual ~DocumentAnalyzerMap(){}

	virtual void addAnalyzer(
			const std::string& mimeType_,
			const std::string& scheme_,
			DocumentAnalyzerInterface* analyzer_);

	virtual analyzer::Document analyze(
			const std::string& content,
			const analyzer::DocumentClass& dclass) const;

	virtual DocumentAnalyzerContextInterface* createContext(
			const analyzer::DocumentClass& dclass) const;

	virtual analyzer::DocumentAnalyzerMapView view() const;

private:
	const DocumentAnalyzerInterface* getAnalyzer( const analyzer::DocumentClass& dclass) const;

private:
	typedef strus::Reference<DocumentAnalyzerInterface> DocumentAnalyzerReference;
	typedef std::map<std::string,const DocumentAnalyzerInterface*> Map;

private:
	ErrorBufferInterface* m_errorhnd;
	Map m_mimeTypeAnalyzerMap;
	Map m_schemeAnalyzerMap;
	std::vector<DocumentAnalyzerReference> m_analyzers;
};

}//namespace
#endif

