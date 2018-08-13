/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_CONTENT_ITERATOR_TEXTWOLF_HPP_INCLUDED
#define _STRUS_SEGMENTER_CONTENT_ITERATOR_TEXTWOLF_HPP_INCLUDED
#include "strus/contentIteratorInterface.hpp"
#include "strus/analyzer/segmenterOptions.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "strus/errorBufferInterface.hpp"
#include "private/contentIteratorStm.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include "textwolf/xmlscanner.hpp"
#include <cstdlib>
#include <vector>
#include <set>
#include <setjmp.h>

#define SEGMENTER_NAME "textwolf"

namespace strus
{

template <class CharsetEncoding>
class ContentIterator
	:public ContentIteratorInterface
{
public:
	ContentIterator( 
			const char* content_,
			std::size_t contentsize_,
			const std::vector<std::string>& attributes_,
			const CharsetEncoding& charset_,
			ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_)
		,m_attributes(attributes_.begin(),attributes_.end())
		,m_content(content_,contentsize_)
		,m_srciter()
		,m_scanner(charset_,textwolf::SrcIterator())
		,m_eom()
		,m_itr(),m_end()
		,m_stm()
		,m_path()
		,m_eof(false)
	{
		m_srciter.putInput( m_content.c_str(), m_content.size(), &m_eom);
		m_scanner.setSource( m_srciter);
		m_itr = m_scanner.begin(false);
		m_end = m_scanner.end();
	}

	virtual ~ContentIterator(){}

	virtual bool getNext(
			const char*& expression, std::size_t& expressionsize,
			const char*& segment, std::size_t& segmentsize)
	{
		try
		{
			if (m_eof || setjmp(m_eom) != 0)
			{
				return false;
			}
			else for (;;)
			{
				++m_itr;
				if (m_itr->type() == textwolf::XMLScannerBase::Exit)
				{
					m_eof = true;
					return false;
				}
				else if (m_itr->type() == textwolf::XMLScannerBase::ErrorOccurred)
				{
					const char* errstr = "";
					m_scanner.getError( &errstr);
					throw strus::runtime_error( _TXT("error in document at position %u: %s"), (unsigned int)m_scanner.getTokenPosition(), errstr);
				}
				else if (m_stm.textwolfItem(
					m_itr->type(), m_itr->content(), m_itr->size(),
					expression, expressionsize, segment, segmentsize))
				{
					return true;
				}
			}
		}
		CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in get next of '%s' content iterator: %s"), SEGMENTER_NAME, *m_errorhnd, false);
	}

private:
	typedef textwolf::XMLScanner<
			textwolf::SrcIterator,
			CharsetEncoding,
			textwolf::charset::UTF8,
			std::string
		> XMLScanner;

	ErrorBufferInterface* m_errorhnd;
	std::set<std::string> m_attributes;
	std::string m_content;
	textwolf::SrcIterator m_srciter;
	XMLScanner m_scanner;
	jmp_buf m_eom;
	typename XMLScanner::iterator m_itr;
	typename XMLScanner::iterator m_end;
	ContentIteratorStm m_stm;
	std::string m_path;
	bool m_eof;
};

}//namespace
#endif


