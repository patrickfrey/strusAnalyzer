/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Parameterizable query analyzer interface
/// \file queryAnalyzerInterface.hpp
#ifndef _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_QUERY_ANALYZER_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/term.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include <vector>
#include <string>

/// \brief strus toplevel namespace
namespace strus
{

/// \brief Defines a program for analyzing chunks of a query
class QueryAnalyzerInterface
{
public:
	/// \brief Destructor
	virtual ~QueryAnalyzerInterface(){}

	/// \brief Declare how a set of query features is produced out from a phrase of a certain type
	/// \param[in] phraseType label of the phrase type
	/// \param[in] featureType type name (in the storage) of the generated features
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (ownership of elements passed to this) to use for this feature
	/// \note It is recommended to name the phraseType as the featureType to avoid to many different namings. The phrase type is used to address the method, as the expressions in the segmenter of the document do. But in most cases there will be only one of a kind, so it does not make sense to have disinct names for it.
	virtual void definePhraseType(
			const std::string& phraseType,
			const std::string& featureType,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers)=0;

	/// \brief Analyze a single phrase of query
	/// \param[in] phraseType selects the feature configuration that determines how the phrase is tokenized and normalized, what types the resulting terms get and what set (as referred to in the query evaluation) the created query features are assigned to.
	/// \param[in] content string of the phrase to analyze
	/// \note The query language determines the segmentation of the query parts.
	virtual std::vector<analyzer::Term> analyzePhrase(
			const std::string& phraseType,
			const std::string& content) const=0;

	/// \brief Definition of a phrase to analyze
	class Phrase
	{
	public:
		/// \brief Constructor
		/// \param[in] type_ the phrase type
		/// \param[in] content_ the phrase content
		Phrase( const std::string& type_, const std::string& content_)
			:m_type(type_),m_content(content_){}
		/// \brief Copy constructor
		Phrase( const Phrase& o)
			:m_type(o.m_type),m_content(o.m_content){}

		/// \brief Get the phrase type
		/// \return the phrase type value
		const std::string& type() const		{return m_type;}
		/// \brief Get the phrase content
		/// \return the phrase content value
		const std::string& content() const	{return m_content;}

	private:
		std::string m_type;
		std::string m_content;
	};

	/// \brief Analyze a bulk of phrases
	/// \param[in] phraseBulk vector of phrase defoinitions to analyze
	/// \return a vector of analyzed phrases, parallel to the passed phrase bulk
	virtual std::vector<analyzer::TermArray> analyzePhraseBulk(
			const std::vector<Phrase>& phraseBulk) const=0;
};

}//namespace
#endif

