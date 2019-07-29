/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Structure describing the library for determining the content type of items in the analysis of a content
/// \file contentStatisticsLibrary.hpp
#ifndef _STRUS_ANALYZER_CONTENT_STATISTICS_LIBRARY_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_STATISTICS_LIBRARY_HPP_INCLUDED
#include "strus/reference.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/base/thread.hpp"
#include "strus/base/regex.hpp"
#include "strus/structView.hpp"
#include <vector>
#include <string>
#include <cstring>
#include <map>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forwrd declaration
class ErrorBufferInterface;

/// \brief Structure used to collect data for content statistics
class ContentStatisticsLibrary
{
public:
	/// \brief Default constructor
	explicit ContentStatisticsLibrary( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_ar(){}
	/// \brief Constructor
	ContentStatisticsLibrary( const ContentStatisticsLibrary& o)
		:m_errorhnd(o.m_errorhnd),m_ar(o.m_ar){}
	
	/// \brief Destructor
	~ContentStatisticsLibrary(){}

	/// \brief Define an attribute to be visible as path info
	/// \param[in] name name of the attribute to collect
	void addVisibleAttribute( const std::string& name);

	/// \brief Check if an attribute is to be collected as path info
	/// \param[in] name name of the candidate attribute
	std::vector<std::string> collectedAttributes() const;

	/// \brief Declare an element of the library used to categorize features
	/// \param[in] type type name of the feature
	/// \param[in] regex regular expression that has to match on the whole segment in order to consider it as candidate
	/// \param[in] priority priority given to matches, for multiple matches only the ones with highest priority are selected
	/// \param[in] minLength minimum length of the chunk or -1 if no restriction
	/// \param[in] maxLength maximum length of the chunk or -1 if no restriction
	/// \param[in] tokenizer tokenizer (ownership passed to this) to use for this feature
	/// \param[in] normalizers list of normalizers (element ownership passed to this) to use for this feature
	void addElement(
			const std::string& type,
			const std::string& regex,
			int priority,
			int minLength,
			int maxLength,
			TokenizerFunctionInstanceInterface* tokenizer,
			const std::vector<NormalizerFunctionInstanceInterface*>& normalizers);

	/// \brief Get the list of matches for an input
	/// \param[in] input input chunk to match
	/// \param[in] inputsize size of input chunk in bytes
	/// \return list of matching types
	std::vector<std::string> matches( const char* input, std::size_t inputsize) const;

	/// \brief Get the list of element views
	StructView view() const;

private:
	typedef strus::Reference<TokenizerFunctionInstanceInterface> TokenizerFunctionReference;
	typedef strus::Reference<NormalizerFunctionInstanceInterface> NormalizerFunctionReference;
	typedef strus::Reference<RegexSearch> RegexSearchReference;

	struct Element
	{
		std::string type;
		std::string regexstr;
		RegexSearchReference regex;
		int priority;
		int minLength;
		int maxLength;
		TokenizerFunctionReference tokenizer;
		std::vector<NormalizerFunctionReference> normalizers;

		Element( const std::string& type_,
				const std::string& regexstr_, const RegexSearchReference& regex_,
				int priority_, int minLength_, int maxLength_,
				const TokenizerFunctionReference& tokenizer_,
				const std::vector<NormalizerFunctionReference>& normalizers_)
			:type(type_),regexstr(regexstr_),regex(regex_),priority(priority_),minLength(minLength_),maxLength(maxLength_),tokenizer(tokenizer_),normalizers(normalizers_){}
		Element( const Element& o)
			:type(o.type),regexstr(o.regexstr),regex(o.regex),priority(o.priority),minLength(o.minLength),maxLength(o.maxLength),tokenizer(o.tokenizer),normalizers(o.normalizers){}
	};

private:
	mutable strus::mutex m_mutex;
	ErrorBufferInterface* m_errorhnd;
	std::vector<Element> m_ar;
	std::vector<std::string> m_attributes;
};

}//namespace
#endif

