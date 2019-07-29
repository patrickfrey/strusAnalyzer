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
#include "strus/structView.hpp"

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a document analyzer for introspection
/// \note The internal representations may not be suitable for reconstructing the objects
class DocumentAnalyzerView
	:public StructView
{
public:
	/// \brief Constructor
	/// \param[in] segmenter_ segmenter
	/// \param[in] subcontents_ sub content definitions
	/// \param[in] subdocuments_ sub document definitions
	/// \param[in] attributes_ attribute definitions
	/// \param[in] metadata_ metadata definitions
	/// \param[in] searchindex_ search index feature definitions
	/// \param[in] forwardindex_ forward index feature definitions
	/// \param[in] aggregators_ aggregator definitions
	/// \param[in] lexems_ lexems defined internally for pattern matching
	DocumentAnalyzerView(
			const StructView& segmenter_,
			const StructView& subcontents_,
			const StructView& subdocuments_,
			const StructView& attributes_,
			const StructView& metadata_,
			const StructView& searchindex_,
			const StructView& forwardindex_,
			const StructView& aggregators_,
			const StructView& lexems_)
	{
		StructView::operator()
			( "segmenter", segmenter_)
			( "subcontent", subcontents_)
			( "subdocument", subdocuments_)
			( "attribute", attributes_)
			( "metadata", metadata_)
			( "searchindex", searchindex_)
			( "forwardindex", forwardindex_)
			( "aggregator", aggregators_)
			( "lexem", lexems_)
		;
	}
};

}}//namespace
#endif

