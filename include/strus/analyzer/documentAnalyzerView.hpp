/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing the internal representation of a document analyzer for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
/// \file documentAnalyzerView.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_ANALYZER_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_ANALYZER_VIEW_HPP_INCLUDED
#include "strus/analyzer/functionView.hpp"
#include "strus/analyzer/featureView.hpp"
#include "strus/analyzer/aggregatorView.hpp"
#include "strus/analyzer/subDocumentDefinitionView.hpp"
#include "strus/analyzer/subContentDefinitionView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a document analyzer for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
class DocumentAnalyzerView
{
public:
	/// \brief Default constructor
	DocumentAnalyzerView(){}
	/// \brief Copy constructor
	DocumentAnalyzerView( const DocumentAnalyzerView& o)
		:m_segmenter(o.m_segmenter)
		,m_subcontents(o.m_subcontents)
		,m_subdocuments(o.m_subdocuments)
		,m_attributes(o.m_attributes)
		,m_metadata(o.m_metadata)
		,m_searchindex(o.m_searchindex)
		,m_forwardindex(o.m_forwardindex)
		,m_aggregators(o.m_aggregators)
		{}
	/// \brief Constructor
	/// \param[in] segmenter_ segmenter
	/// \param[in] subcontents_ sub content definitions
	/// \param[in] subdocuments_ sub document definitions
	/// \param[in] attributes_ attribute definitions
	/// \param[in] metadata_ metadata definitions
	/// \param[in] searchindex_ search index feature definitions
	/// \param[in] forwardindex_ forward index feature definitions
	/// \param[in] aggregators_ aggregator definitions
	DocumentAnalyzerView(
			const FunctionView& segmenter_,
			const std::vector<SubContentDefinitionView>& subcontents_,
			const std::vector<SubDocumentDefinitionView>& subdocuments_,
			const std::vector<FeatureView>& attributes_,
			const std::vector<FeatureView>& metadata_,
			const std::vector<FeatureView>& searchindex_,
			const std::vector<FeatureView>& forwardindex_,
			const std::vector<AggregatorView>& aggregators_)
		:m_segmenter(segmenter_)
		,m_subcontents(subcontents_)
		,m_subdocuments(subdocuments_)
		,m_attributes(attributes_)
		,m_metadata(metadata_)
		,m_searchindex(searchindex_)
		,m_forwardindex(forwardindex_)
		,m_aggregators(aggregators_)
		{}

	const FunctionView& segmenter() const					{return m_segmenter;}
	const std::vector<SubContentDefinitionView>& subcontents() const	{return m_subcontents;}
	const std::vector<SubDocumentDefinitionView>& subdocuments() const	{return m_subdocuments;}
	const std::vector<FeatureView>& attributes() const			{return m_attributes;}
	const std::vector<FeatureView>& metadata() const			{return m_metadata;}
	const std::vector<FeatureView>& searchindex() const			{return m_searchindex;}
	const std::vector<FeatureView>& forwardindex() const			{return m_forwardindex;}
	const std::vector<AggregatorView>& aggregators() const			{return m_aggregators;}

private:
	FunctionView m_segmenter;
	std::vector<SubContentDefinitionView> m_subcontents;
	std::vector<SubDocumentDefinitionView> m_subdocuments;
	std::vector<FeatureView> m_attributes;
	std::vector<FeatureView> m_metadata;
	std::vector<FeatureView> m_searchindex;
	std::vector<FeatureView> m_forwardindex;
	std::vector<AggregatorView> m_aggregators;
};

}}//namespace
#endif

