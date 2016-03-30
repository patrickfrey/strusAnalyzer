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
#include "strus/documentClass.hpp"
#include "strus/documentClassDetectorInterface.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

class StandardDocumentClassDetector
	:public DocumentClassDetectorInterface
{
public:
	explicit StandardDocumentClassDetector( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}
	virtual ~StandardDocumentClassDetector(){}

	virtual bool detect( DocumentClass& dclass, const char* contentBegin, std::size_t contentBeginSize) const;

private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif


