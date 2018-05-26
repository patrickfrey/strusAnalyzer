/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for document content statistics
/// \file contentStatisticsInterface.hpp
#ifndef _STRUS_ANALYZER_CONTENT_STATISTICS_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_STATISTICS_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/contentStatisticsItem.hpp"
#include "strus/analyzer/contentStatisticsView.hpp"
#include "strus/contentStatisticsContextInterface.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Forward declaration
class NormalizerFunctionInstanceInterface;
/// \brief Forward declaration
class TokenizerFunctionInstanceInterface;
/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class TextProcessorInterface;

/// \brief Defines a program for analyzing a document, splitting it into normalized terms that can be fed to the strus IR engine
class ContentStatisticsInterface
{
public:
	/// \brief Destructor
	virtual ~ContentStatisticsInterface(){}

	/// \brief Declare an element of the library used to categorize features
	/// \param[in] type type name of the feature
	/// \param[in] regex regular expression that has to match on the whole segment in order to consider it as candidate
	/// \param[in] minLength minimum length of the chunk or -1 if no restriction
	/// \param[in] maxLength maximum length of the chunk or -1 if no restriction
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	virtual void addLibraryElement(
			const std::string& type,
			const std::string& regex,
			int minLength,
			int maxLength,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Create the context used for collecting document statitics
	/// \return the document content statistics context (with ownership)
	virtual ContentStatisticsContextInterface* createContext() const=0;

	/// \brief Return a structure with all definitions for introspection
	/// \return the structure with all definitions for introspection
	virtual analyzer::ContentStatisticsView view() const=0;
};

}//namespace
#endif

