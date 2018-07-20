/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation for annotation of text in one document
/// \file "tokenMarkup.cpp"
#include "tokenMarkup.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/segmenterMarkupContextInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/base/local_ptr.hpp"
#include <algorithm>
#include <memory>

using namespace strus;

TokenMarkupContext::TokenMarkupContext(
		const SegmenterInstanceInterface* segmenter_,
		ErrorBufferInterface* errorhnd_)
	:m_segmenter(segmenter_),m_errorhnd(errorhnd_)
{}

TokenMarkupContext::~TokenMarkupContext()
{}

void TokenMarkupContext::putMarkup(
		const analyzer::Position& start,
		const analyzer::Position& end,
		const analyzer::TokenMarkup& markup,
		unsigned int level)
{
	try
	{
		m_markupar.push_back( MarkupElement( start, end, markup, level, m_markupar.size()));
	}
	CATCH_ERROR_MAP( _TXT("failed to put token markup in document: %s"), *m_errorhnd);
}

void TokenMarkupContext::writeOpenMarkup( SegmenterMarkupContextInterface* markupdoc, const SegmenterPosition& segpos, std::size_t ofs, const analyzer::TokenMarkup& markup)
{
	if (!markup.name().empty())
	{
		markupdoc->putOpenTag( segpos, ofs, markup.name());
	}
	std::vector<analyzer::TokenMarkup::Attribute>::const_iterator
		ai = markup.attributes().begin(), ae = markup.attributes().end();
	for (; ai != ae; ++ai)
	{
		markupdoc->putAttribute( segpos, ofs, ai->name(), ai->value());
	}
}

std::string TokenMarkupContext::markupDocument(
		const analyzer::DocumentClass& dclass,
		const std::string& content) const
{
	try
	{
		strus::local_ptr<SegmenterMarkupContextInterface> markupdoc( m_segmenter->createMarkupContext( dclass, content));
		if (!markupdoc.get()) throw std::runtime_error( _TXT("failed to create markup document context"));

		std::vector<MarkupElement> markupar = m_markupar;
		std::sort( markupar.begin(), markupar.end());
		std::vector<MarkupElement>::iterator mp = markupar.begin(), mi = markupar.begin(), me = markupar.end();
		// Resolve conflicts:
		for (++mi; mi != me; mp=mi,++mi)
		{
			if (mp->end.seg() > mi->end.seg() || (mp->end.seg() == mi->end.seg() && mp->end.ofs() >= mi->end.ofs()))
			{
				//... previous overlaps current, resolve conflict:
				if (mp->level >= mi->level)
				{
					MarkupElement newelem( *mp);
					newelem.start = mi->end;
					mp->end = mi->start;
					if (newelem.end > newelem.start)
					{
						mi = markupar.insert( mi+1, newelem);
						--mi;
						--mi;
					}
				}
				else// if (mp->level < mi->level)
				{
					mi->start = mp->end;
				}
			}
		}
		// Insert markups:
		mi = markupar.begin(), me = markupar.end();
		for (; mi != me; ++mi)
		{
			writeOpenMarkup( markupdoc.get(), mi->start.seg(), mi->start.ofs(), mi->markup);
			if (mi->start.seg() != mi->end.seg())
			{
				// If markup is overlapping more than one segment, we iterate through the 
				// touched segments and insert a close markup at every end of a touched 
				// segment a reopen the markup at the start of the follow segment:
				SegmenterPosition itr_segpos = mi->start.seg();
				std::size_t itr_size = markupdoc->segmentSize( itr_segpos);
				while (itr_segpos != mi->end.seg())
				{
					markupdoc->putCloseTag( itr_segpos, itr_size, mi->markup.name());
					const char* segment;
					if (!markupdoc->getNext( itr_segpos, segment, itr_size))
					{
						throw strus::runtime_error( "%s",  _TXT("cannot find path to end of markup"));
					}
					writeOpenMarkup( markupdoc.get(), itr_segpos, 0, mi->markup);
				}
			}
			markupdoc->putCloseTag( mi->end.seg(), mi->end.ofs(), mi->markup.name());
		}
		return markupdoc->getContent();
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to create document with markups inserted: %s"), *m_errorhnd, std::string());
}

TokenMarkupContextInterface* TokenMarkupInstance::createContext( const SegmenterInstanceInterface* segmenter) const
{
	try
	{
		return new TokenMarkupContext( segmenter, m_errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("failed to create token markup: %s"), *m_errorhnd, 0);
}

analyzer::FunctionView TokenMarkupInstance::view() const
{
	try
	{
		return analyzer::FunctionView( "std")
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, analyzer::FunctionView());
}

