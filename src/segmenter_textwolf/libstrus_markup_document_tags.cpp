/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Library for adding attributes to selected tags of a document (currently only implemented for XML)
/// \file libstrus_attribute_tags.cpp
#include "strus/analyzer/lib/attribute_tags.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/xpathAutomaton.hpp"
#include "private/textEncoder.hpp"
#include "private/contentIteratorStm.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"

#define SEGMENTER_NAME "textwolf"
#define MODULE_NAME "markup document tags"

using namespace strus;

template <class CharsetEncoding>
class MarkupInput
{
public:
	MarkupInput( ErrorBufferInterface* errorhnd_, const char* src_, std::size_t srcsize_, const XPathAutomaton* automaton_, const CharsetEncoding& charset_=CharsetEncoding())
		:m_automaton(automaton_)
		,m_xpathselect(automaton_->createContext())
		,m_srciter()
		,m_scanner(charset_,textwolf::SrcIterator())
		,m_itr()
		,m_end()
		,m_eof(false)
		,m_eom()
		,m_src(src_)
		,m_srcsize(srcsize_)
		,m_errorhnd(errorhnd_)
	{
		m_srciter.putInput( m_src, m_srcsize, &m_eom);
		m_scanner.setSource( m_srciter);
		m_itr = m_scanner.begin(false);
		m_end = m_scanner.end();
	}

	struct Element
	{
		typename XMLScanner::ElementType type;
		const char* ptr;
		std::size_t size;

		explicit Element( typename XMLScanner::ElementType type_)
			:type(type_),ptr(0),size(0){}
		Element( typename XMLScanner::ElementType type_, const char* ptr_, std::size_t size_)
			:type(type_),ptr(ptr_),size(size_){}
		Element( const Element& o)
			:type(o.type),ptr(o.ptr),size(o.size){}
	};

	Element fetch()
	{
		if (setjmp(m_eom) != 0)
		{
			if (!m_eof) m_errorhnd->report( ErrorCodeUnexpectedEof, _TXT( "unexpected end of input in '%s' %s"), SEGMENTER_NAME, MODULE_NAME);
			return false;
		}
		++m_itr;

		typename XMLScanner::ElementType et = m_itr->type();
		if (et == XMLScanner::ErrorOccurred)
		{
			const char* errstr = "";
			m_scanner.getError( &errstr);
			m_errorhnd->report( ErrorCodeSyntax, _TXT("error in document at position %u: %s"), (unsigned int)m_scanner.getTokenPosition(), errstr);
			return Element( XMLScanner::ErrorOccurred);
		}
		else if (et == XMLScanner::Exit)
		{
			m_eof = true;
			return Element( XMLScanner::Exit);
		}
		
		pos = m_scanner.getTokenPosition();
		segment = m_itr->content();
		segmentsize = m_itr->size();
		return true;
	}

private:
	typedef textwolf::XMLScanner<
			textwolf::SrcIterator,
			CharsetEncoding,
			textwolf::charset::UTF8,
			std::string
		> XMLScanner;

	const XPathAutomaton* m_automaton;
	XPathAutomatonContext m_xpathselect;
	textwolf::SrcIterator m_srciter;
	XMLScanner m_scanner;
	typename XMLScanner::iterator m_itr;
	typename XMLScanner::iterator m_end;
	bool m_eof;
	jmp_buf m_eom;
	const char* m_src;
	std::size_t m_srcsize;
	ErrorBufferInterface* m_errorhnd;
};

std::string markupDocumentTags( const std::string& content, const analyzer::DocumentClass& documentClass, const std::string& selectexpr, TagAttributeMarkupInterface* markup, ErrorBufferInterface* errhnd)
{
	
}


