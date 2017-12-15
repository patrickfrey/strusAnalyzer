/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "segmenterContext.hpp"
#include "strus/errorBufferInterface.hpp"
#include "segmenter.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstdlib>
#include <cstring>

using namespace strus;

#define SEGMENTER_NAME "plain"

void SegmenterContext::putInput( const char* chunk, std::size_t chunksize, bool eof)
{
	try
	{
		m_content.append( chunk, chunksize);
		if (eof)
		{
			m_eof = true;
			if (m_encoder.get())
			{
				m_content = m_encoder->convert( m_content.c_str(), m_content.size(), true);
			}
		}
	}
	CATCH_ERROR_MAP_ARG1( _TXT("error in put input of %s segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}

bool SegmenterContext::getNext( int& id, SegmenterPosition& pos, const char*& segment, std::size_t& segmentsize)
{
	if (!m_eof) return false;
	if (m_iditr == m_ids->end()) return false;
	id = *m_iditr++;
	pos = 0;
	segment = m_content.c_str();
	segmentsize = m_content.size();
	return true;
}


