/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Standard document class detector declaration
/// \file detectorStd.hpp
#ifndef _STRUS_ANALYZER_STANDARD_DOCUMENT_DETECTOR_HPP_INCLUDED
#define _STRUS_ANALYZER_STANDARD_DOCUMENT_DETECTOR_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include "strus/reference.hpp"
#include "strus/documentClassDetectorInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include <string>
#include <vector>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceContextInterface;
/// \brief Forward declaration
class TextProcessorInterface;

class StandardDocumentClassDetector
	:public DocumentClassDetectorInterface
{
public:
	StandardDocumentClassDetector( const TextProcessorInterface* textproc_, ErrorBufferInterface* errorhnd_);
	virtual ~StandardDocumentClassDetector(){}

	virtual void defineDocumentSchemaDetector(
			const std::string& schema,
			const std::string& mimeType,
			const std::vector<std::string>& select_expressions,
			const std::vector<std::string>& reject_expressions);

	virtual bool detect(
			analyzer::DocumentClass& dclass,
			const char* contentBegin, std::size_t contentBeginSize,
			bool isComplete) const;

private:
	SegmenterInstanceInterface* getSegmenterInstance( const std::string& mimeType);
	int detectSchema( const SegmenterInstanceInterface* segmenter, analyzer::DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize, bool isComplete) const;

private:
	struct SchemaDef
	{
		std::string name;
		std::vector<std::string> select_expressions;
		std::vector<std::string> reject_expressions;

		SchemaDef( const std::string& name_, const std::vector<std::string>& select_expressions_, const std::vector<std::string>& reject_expressions_)
			:name(name_),select_expressions(select_expressions_),reject_expressions(reject_expressions_){}
		SchemaDef( const SchemaDef& o)
			:name(o.name),select_expressions(o.select_expressions),reject_expressions(o.reject_expressions){}

		int nofEvents() const	{return select_expressions.size();}
	};

	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
	const TextProcessorInterface* m_textproc;
	std::vector<SchemaDef> m_schemas;
	strus::Reference<SegmenterInstanceInterface> m_xmlSegmenter;
	strus::Reference<SegmenterInstanceInterface> m_jsonSegmenter;
	strus::Reference<SegmenterInstanceInterface> m_tsvSegmenter;
};

}//namespace
#endif


