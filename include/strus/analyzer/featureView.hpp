/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structures describing the internal representation of a feature in a document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
/// \file featureView.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_FEATURE_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_FEATURE_VIEW_HPP_INCLUDED
#include "strus/analyzer/functionView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a feature in the document analyzer
/// \note The internal representation may not be suitable for reconstructing the object
class FeatureView
{
public:
	/// \brief Default constructor
	FeatureView(){}
	/// \brief Copy constructor
	FeatureView( const FeatureView& o)
		:m_type(o.m_type),m_selectexpr(o.m_selectexpr),m_tokenizer(o.m_tokenizer),m_normalizer(o.m_normalizer),m_options(o.m_options){}

	/// \brief Constructor
	/// \param[in] name_ name of the function
	/// \param[in] params_ list of named parameters
	FeatureView( const std::string& type_, const std::string& selectexpr_, const FunctionView& tokenizer_, const std::vector<FunctionView>& normalizer_, const FeatureOptions& options_)
		:m_type(type_),m_selectexpr(selectexpr_),m_tokenizer(tokenizer_),m_normalizer(normalizer_),m_options(options_){}

	/// \brief Get the type
	const std::string& type() const				{return m_type;}
	/// \brief Get the segmenter selection expression
	const std::string& selectexpr() const			{return m_selectexpr;}
	/// \brief Get the tokenizer
	const FunctionView& tokenizer() const			{return m_tokenizer;}
	/// \brief Get the list of normalizers
	const std::vector<FunctionView>& normalizer() const	{return m_normalizer;}
	/// \brief Get the options
	const FeatureOptions& options() const			{return m_options;}

	/// \brief Get position bind option
	const char* posbindOption() const
	{
		static const char* ar[] = {"content","succ","pred","unique"};
		return ar[ (int)m_options.positionBind()];
	}

private:
	std::string m_type;
	std::string m_selectexpr;
	FunctionView m_tokenizer;
	std::vector<FunctionView> m_normalizer;
	FeatureOptions m_options;
};

}}//namespace
#endif

