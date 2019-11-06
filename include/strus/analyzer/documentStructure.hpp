/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure in a document defined as directed binary relation of a positional range (called header) to another positional range (called content)
/// \file document.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_STRUCTURE_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_STRUCTURE_HPP_INCLUDED
#include <string>
#include <utility>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure in a document defined as directed binary relation of a positional range (called header) to another positional range (called content)
class DocumentStructure
{
public:
	typedef int Position;
	typedef std::pair<Position,Position> PositionRange;

	DocumentStructure(
			const std::string& name_,
			const PositionRange& source_,
			const PositionRange& sink_)
		:m_name(name_),m_source(source_),m_sink(sink_){}
	DocumentStructure(
			const DocumentStructure& o)
		:m_name(o.m_name),m_source(o.m_source),m_sink(o.m_sink){}

private:
	std::string m_name;
	PositionRange m_source;
	PositionRange m_sink;
};
}}//namespace
#endif


