/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "contentIterator.hpp"
#include "cjson2textwolf.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/errorCodes.hpp"
#include "private/contentIteratorStm.hpp"
#include "textwolf/xmlscanner.hpp"
#include "cjson/cJSON.h"
#include <cstdlib>
#include <list>
#include <setjmp.h>

#define SEGMENTER_NAME "cjson"

using namespace strus;

ContentIterator::ContentIterator( 
		const char* content_,
		std::size_t contentsize_,
		const strus::Reference<strus::utils::TextEncoderBase>& encoder_,
		ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_)
	,m_content()
	,m_encoder(encoder_)
	,m_ar()
	,m_tree(0)
	,m_stm()
	,m_path()
	,m_eof(false)
{
	try
	{
		if (m_encoder.get())
		{
			m_content = m_encoder->convert( content_, contentsize_, true);
		}
		else
		{
			m_content = std::string( content_, contentsize_);
		}
		m_tree = strus::parseJsonTree( m_content);

		strus::getTextwolfItems( m_ar, m_tree);
		m_elemitr = m_ar.begin();
	}
	catch (const std::bad_alloc&)
	{
		clear();
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory creating JSON content iterator"));
	}
	catch (const std::runtime_error& err)
	{
		clear();
		m_errorhnd->report( ErrorCodeRuntimeError, _TXT("error creating JSON content iterator: %s"), err.what());
	}
}

void ContentIterator::clear()
{
	if (m_tree) cJSON_Delete( m_tree);
}

bool ContentIterator::getNext(
		const char*& expression, std::size_t& expressionsize,
		const char*& segment, std::size_t& segmentsize)
{
	try
	{
		for (;;)
		{
			if (m_elemitr == m_ar.end()) return false;
	
			if (m_elemitr->type == textwolf::XMLScannerBase::Exit)
			{
				m_eof = true;
				return false;
			}
			else if (m_elemitr->type == textwolf::XMLScannerBase::ErrorOccurred)
			{
				throw strus::runtime_error( _TXT("error in document: %s"), m_elemitr->value);
			}
			else if (m_stm.textwolfItem(
					m_elemitr->type, m_elemitr->value, m_elemitr->value?std::strlen(m_elemitr->value):0,
					expression, expressionsize, segment, segmentsize))
			{
				return true;
			}
			++m_elemitr;
		}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in content iterator of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, false);
}

