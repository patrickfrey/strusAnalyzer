/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "segmenter.hpp"
#include "segmenterContext.hpp"
#include "strus/documentClass.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"

using namespace strus;

#define SEGMENTER_NAME "cjson"

void SegmenterInstance::defineSelectorExpression( int id, const std::string& expression)
{
	try
	{
		m_automaton.defineSelectorExpression( id, expression);
	}
	CATCH_ERROR_MAP_ARG1( _TXT("error defining expression for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}


void SegmenterInstance::defineSubSection( int startId, int endId, const std::string& expression)
{
	try
	{
		m_automaton.defineSubSection( startId, endId, expression);
	}
	CATCH_ERROR_MAP_ARG1( _TXT("error defining subsection for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}


SegmenterContextInterface* SegmenterInstance::createContext( const DocumentClass& dclass) const
{
	try
	{
		return new SegmenterContext( m_errorhnd, &m_automaton);
	}
	CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

SegmenterInstanceInterface* Segmenter::createInstance() const
{
	try
	{
		return new SegmenterInstance( m_errorhnd);
	}
	CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}


