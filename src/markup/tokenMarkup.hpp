/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation for annotation of spans in text
/// \file "tokenMarkup.hpp"
#ifndef _STRUS_STREAM_TOKEN_MARKUP_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_STREAM_TOKEN_MARKUP_IMPLEMENTATION_HPP_INCLUDED
#include "strus/tokenMarkupContextInterface.hpp"
#include "strus/tokenMarkupInstanceInterface.hpp"
#include "strus/analyzer/tokenMarkup.hpp"
#include "strus/analyzer/functionView.hpp"

namespace strus
{
/// \brief Forward declaration
class SegmenterMarkupContextInterface;
/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Implementation for annotation of text in one document
class TokenMarkupContext
	:public TokenMarkupContextInterface
{
public:
	/// \brief Constructor
	/// \param[in] errorhnd_ error buffer interface
	explicit TokenMarkupContext(
			ErrorBufferInterface* errorhnd_);

	/// \brief Destructor
	virtual ~TokenMarkupContext();

	virtual void putMarkup(
			const analyzer::Position& start,
			const analyzer::Position& end,
			const analyzer::TokenMarkup& markup,
			unsigned int level);

	virtual std::string markupDocument(
			const SegmenterInstanceInterface* segmenter,
			const analyzer::DocumentClass& dclass,
			const std::string& content) const;

private:
	static void writeOpenMarkup(
			SegmenterMarkupContextInterface* markupdoc,
			const SegmenterPosition& segpos, std::size_t ofs, const analyzer::TokenMarkup& markup);

private:
	struct MarkupElement
	{
		analyzer::Position start;		///< start of the item to mark
		analyzer::Position end;			///< end of the item to mark
		analyzer::TokenMarkup markup;		///< tag for markup in document
		unsigned int level;			///< level deciding what markup superseds others when they are overlapping
		unsigned int orderidx;			///< index to keep deterministic, stable ordering in sorted array

		MarkupElement( const analyzer::Position& start_, const analyzer::Position& end_, const analyzer::TokenMarkup& markup_, unsigned int level_, unsigned int orderidx_)
			:start(start_),end(end_),markup(markup_),level(level_),orderidx(orderidx_){}
#if __cplusplus >= 201103L
		MarkupElement( MarkupElement&& ) = default;
		MarkupElement( const MarkupElement& ) = default;
		MarkupElement& operator= ( MarkupElement&& ) = default;
		MarkupElement& operator= ( const MarkupElement& ) = default;
#else
		MarkupElement( const MarkupElement& o)
			:start(o.start),end(o.end),markup(o.markup),level(o.level),orderidx(o.orderidx){}
#endif

		bool operator<( const MarkupElement& o) const
		{
			if (start < o.start) return true;
			if (start > o.start) return false;
			if (end < o.end) return true;
			if (end > o.end) return false;
			if (level < o.level) return true;
			if (level > o.level) return false;
			if (orderidx < o.orderidx) return true;
			if (orderidx > o.orderidx) return false;
			return false;
		}
	};

private:
	std::vector<MarkupElement> m_markupar;
	ErrorBufferInterface* m_errorhnd;
};


class TokenMarkupInstance
	:public TokenMarkupInstanceInterface
{
public:
	/// \brief Constructor
	explicit TokenMarkupInstance( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	/// \brief Destructor
	virtual ~TokenMarkupInstance(){}

	virtual TokenMarkupContextInterface* createContext() const;

	virtual analyzer::FunctionView view() const;

private:
	ErrorBufferInterface* m_errorhnd;
};

} //namespace
#endif

