/*
 * Copyright (C) 2016 Andreas Baumann
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tsvSegmenter.hpp"
#include "strus/base/string_conv.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "strus/errorBufferInterface.hpp"

#undef STRUS_LOWLEVEL_DEBUG

#ifdef STRUS_LOWLEVEL_DEBUG
#include <iostream>
#endif

#include <algorithm>
#include <cstring>
#include <iterator>

#define SEGMENTER_NAME "tsv"

using namespace strus;

// TSVParserDefinition

TSVParserDefinition::TSVParserDefinition( )
	: m_map( ), m_it( m_map.end( ) ), m_end( m_map.end( ) ),
	m_startId( 0 ), m_endId( 0 )
{
}

void TSVParserDefinition::printDefinitions( )
{
#ifdef STRUS_LOWLEVEL_DEBUG
	std::cout << "DEBUG: definition multimap contains: ";
	for( std::multimap<std::string, int>::const_iterator it = m_map.begin( ); it != m_map.end( ); it++ ) {
		std::cout << "[" << it->first << ", " << it->second << "], ";
	}
	std::cout << std::endl;
#endif
}

void TSVParserDefinition::defineSelectorExpression( int id, const std::string &expression )
{
#ifdef STRUS_LOWLEVEL_DEBUG	
	std::cout << "DEBUG: adding selector expression: " << id << ", " << expression << std::endl;
#endif
	m_map.insert( std::make_pair( expression, id ) );

#ifdef STRUS_LOWLEVEL_DEBUG	
	printDefinitions( );
#endif
}

void TSVParserDefinition::defineSubSection( int startId, int endId, const std::string &expression )
{
#ifdef STRUS_LOWLEVEL_DEBUG	
	std::cout << "DEBUG: adding subsection expression: [" << startId << ", " << endId << "]: " << expression << std::endl;
#endif
	// TODO: assume "line" as selector, currently we don't want to support something
	// like partial lines or combined/runaway lines, so basically we ignore the
	// expression
	m_startId = startId;
	m_endId = endId;
}

int TSVParserDefinition::getNextId( const std::string &definition )
{
#ifdef STRUS_LOWLEVEL_DEBUG	
	std::cout << "DEBUG: lookup expression for: " << definition << std::endl;
#endif
	int id = 0;
	if( m_it != m_map.end( ) ) {
		m_it++;
		if( m_it != m_end ) {
			id = m_it->second;
		} else {
			m_it = m_map.end( );
			m_end = m_map.end( );
		}
	} else {		
		std::pair <std::multimap<std::string, int>::const_iterator, std::multimap<std::string, int>::const_iterator> ret;
		ret = m_map.equal_range( definition );
		if( ret.first == ret.second ) {
			m_it = m_map.end( );
			m_end = m_map.end( );
		} else if( ret.first->first == definition ) {
			m_it = ret.first;
			m_end = ret.second;
			id = m_it->second;
		} else {
			m_it = m_map.end( );
			m_end = m_map.end( );
		}
	}
#ifdef STRUS_LOWLEVEL_DEBUG	
	std::cout << "DEBUG: got number for '" << definition << "' to be " << id << std::endl;
#endif
	return id;
}

bool TSVParserDefinition::moreOftheSame( )
{
#ifdef STRUS_LOWLEVEL_DEBUG	
	std::cout << "DEBUG: moreOftheSame is " <<  ( m_it != m_map.end( ) && m_it != m_end ) << std::endl;
#endif
	return m_it != m_map.end( ) && m_it != m_end;
}

// TSVSegmenterContext

TSVSegmenterContext::TSVSegmenterContext( const TSVParserDefinition& parserDefinition, const strus::Reference<strus::utils::TextEncoderBase>& encoder_, strus::ErrorBufferInterface *errbuf, bool errorReporting )
	: m_errbuf( errbuf ), m_errorReporting( errorReporting), m_buf( ),
	m_eof( false ), m_is( m_buf ), m_currentLine( ),
	m_parserDefinition( parserDefinition ), m_data( ), m_pos( -3 ), m_linepos( 0 ),
	m_parseState( TSV_PARSE_STATE_HEADER ), m_headers( ), m_encoder(encoder_)
{
}

TSVSegmenterContext::~TSVSegmenterContext( )
{
}

void TSVSegmenterContext::putInput( const char *chunk, std::size_t chunksize, bool eof )
{
	try
	{	
#ifdef STRUS_LOWLEVEL_DEBUG	
	std::cout << "DEBUG: putInput '" << chunksize << " (eof: " << eof << ")" << std::endl;
#endif
	
	if( m_eof ) {
		m_errbuf->report( ErrorCodeOperationOrder, _TXT("fed chunk after declared end of input" ));
		return;
	}
	if (m_encoder.get())
	{
		m_buf.append( m_encoder->convert( chunk, chunksize, eof ));
	}
	else
	{
		m_buf.append( chunk, chunksize );
	}
	m_eof |= eof;
	
	m_is.str( m_buf );
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error in put input of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errbuf);
}

static std::vector<std::string> splitLine( const std::string &s, const std::string &delimiter, bool keepEmpty )
{
	std::vector<std::string> result; 

	if( delimiter.empty( ) ) { 
		result.push_back( s );
		return result;
	}

	std::string::const_iterator b = s.begin( ), e;
	while( true ) {
		e = std::search( b, s.end( ), delimiter.begin( ), delimiter.end( ) );
		std::string tmp( b, e );
		if( keepEmpty || !tmp.empty( ) ) {
			result.push_back( tmp );
		}
		if( e == s.end( ) ) {
			break;
		}
		b = e + delimiter.size( );
	}

	return result;
}

bool TSVSegmenterContext::parseHeader( )
{
	m_headers = splitLine( m_currentLine, "\t", true );
	return true;
}

bool TSVSegmenterContext::parseData( int &id, strus::SegmenterPosition &pos, const char *&segment, std::size_t &segmentsize )
{
	// start of line, emit start of section
	if( m_pos == -2 ) {
		id = m_parserDefinition.getStartId( );
		if( id != 0 ) {
			m_pos++;
			return true;
		}
		
		// no subsection definition, continue
		m_pos++;
	}

	// hard-coded field line number (not present in the TSV file itself)
	if( m_pos == -1 ) {
		id = m_parserDefinition.getNextId( "lineno" );
		if( id != 0 ) {
			if( !m_parserDefinition.moreOftheSame( ) ) {
				m_pos++;
			}
			pos = m_linepos * m_headers.size( );
			snprintf( m_lineposStr, 11, "%d", m_linepos );
			segment = m_lineposStr;
			segmentsize = strlen( m_lineposStr );
			return true;
		}

		if( !m_parserDefinition.moreOftheSame( ) ) {
			m_pos++;
		}
	}

	if( m_pos == 0 ) {
		m_data = splitLine( m_currentLine, "\t", true );
	}
		
	do {
		std::string headerName = m_headers[m_pos];
#ifdef STRUS_LOWLEVEL_DEBUG	
		std::cout << "DEBUG: field " << m_linepos << ":" << m_pos << "'" << headerName << "': '" << m_data[m_pos] << "'" << std::endl;
#endif
		
		id = m_parserDefinition.getNextId( headerName );
		
		if( id == 0 ) {
			m_pos++;
		}
		
	} while( id == 0 && (size_t)m_pos < m_headers.size( ) );

	if( id != 0 ) {
		// TODO: think whether we should not return the character position into the TSV here
		pos = m_linepos * m_headers.size( ) + m_pos;
		segment = m_data[m_pos].c_str( );
		segmentsize = m_data[m_pos].size( );
		if( !m_parserDefinition.moreOftheSame( ) ) {
			m_pos++;
		}
		if( (size_t)m_pos >= m_headers.size( ) ) {
			m_pos = -3;
			id = m_parserDefinition.getEndId( );
			if( id != 0 ) {
				return true;
			}
		}
		return true;
	}

	m_pos++;
	if( (size_t)m_pos >= m_headers.size( ) ) {
		m_pos = -3;
		id = m_parserDefinition.getEndId( );
		if( id != 0 ) {
			return true;
		}
	}
	
	return false;
}

bool TSVSegmenterContext::getNext( int &id, strus::SegmenterPosition &pos, const char *&segment, std::size_t &segmentsize )
{
	try
	{
NEXTLINE:
	
	bool hasNext = false;
	switch( m_parseState ) {
		case TSV_PARSE_STATE_HEADER:
			std::getline( m_is, m_currentLine );
			if( m_is.eof( ) ) {
				// we don't have a complete line to work with, so wait for more data
				return false;
			}
			if( !( hasNext = parseHeader( ) ) ) {
				m_parseState = TSV_PARSE_STATE_EOF;
				return false;
			}
			m_parseState = TSV_PARSE_STATE_DATA;
			goto NEXTLINE;
			
		case TSV_PARSE_STATE_DATA:
			if( m_pos < -2 ) {
				std::getline( m_is, m_currentLine );
				if( m_is.eof( ) ) {
					if( m_eof ) {
						m_parseState = TSV_PARSE_STATE_EOF;
						return false;
					}
					// we don't have a complete line to work with, so wait for more data
#ifdef STRUS_LOWLEVEL_DEBUG	
					std::cout << "DEBUG: buffer reset, rest: " << m_currentLine << std::endl;
#endif
					m_data.clear( );
					m_buf.clear( );
					m_buf.append( m_currentLine );
					m_is.clear( );
					m_is.str( m_buf );
					return false;
				}
				m_linepos++;
				m_pos = -2;
			}
				
			if( ( hasNext = parseData( id, pos, segment, segmentsize ) ) ) {
				return hasNext;
			} else {
				goto NEXTLINE;
			}
			break;

		case TSV_PARSE_STATE_EOF:
#ifdef STRUS_LOWLEVEL_DEBUG
			std::cout << "DEBUG: end of data" << std::endl;
#endif
			return false;
	}
	return false;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in get next of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errbuf, false);
}

// TSVSegmenterInstance

TSVSegmenterInstance::TSVSegmenterInstance( strus::ErrorBufferInterface *errbuf, bool errorReporting )
	: m_errbuf( errbuf ), m_errorReporting( errorReporting ), m_parserDefinition( )
{
}

void TSVSegmenterInstance::defineSelectorExpression( int id, const std::string &expression )
{
	try
	{
	m_parserDefinition.defineSelectorExpression( id, expression );
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error defining selector expression of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errbuf);
}
		
void TSVSegmenterInstance::defineSubSection( int startId, int endId, const std::string &expression )
{
	try
	{
		m_parserDefinition.defineSubSection( startId, endId, expression );
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error definint subsection of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errbuf);
}

strus::SegmenterContextInterface* TSVSegmenterInstance::createContext( const strus::analyzer::DocumentClass &dclass ) const
{
	try
	{
	strus::Reference<strus::utils::TextEncoderBase> encoder;
	if (dclass.defined() && !strus::caseInsensitiveEquals( dclass.encoding(), "utf-8"))
	{
		encoder.reset( strus::utils::createTextEncoder( dclass.encoding().c_str()));
	}
	return new TSVSegmenterContext( m_parserDefinition, encoder, m_errbuf, m_errorReporting );
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating context of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errbuf, 0);
}

strus::SegmenterMarkupContextInterface* TSVSegmenterInstance::createMarkupContext( const strus::analyzer::DocumentClass& dclass, const std::string& content) const
{
	try
	{
	m_errbuf->report( ErrorCodeNotImplemented, _TXT("document markup not implemented for '%s' segmenter"), SEGMENTER_NAME);
	return 0;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating markup instance of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errbuf, 0);
}

strus::analyzer::FunctionView TSVSegmenterInstance::view() const
{
	try
	{
		return analyzer::FunctionView( SEGMENTER_NAME)
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errbuf, strus::analyzer::FunctionView());
}

// TSVSegmenter

TSVSegmenter::TSVSegmenter( strus::ErrorBufferInterface *errbuf, bool errorReporting )
	: m_errbuf( errbuf ), m_errorReporting( errorReporting )
{
}

const char* TSVSegmenter::mimeType( ) const
{
	return "text/tab-separated-values";
}

strus::SegmenterInstanceInterface* TSVSegmenter::createInstance( const strus::analyzer::SegmenterOptions& opts) const
{
	try
	{
	if (!opts.items().empty()) throw strus::runtime_error(_TXT("no options defined for segmenter '%s'"), SEGMENTER_NAME);
	return new TSVSegmenterInstance( m_errbuf, m_errorReporting );
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating instance of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errbuf, 0);
}

const char* TSVSegmenter::getDescription() const
{
	return _TXT("Segmenter for TSV (text/tab-separated-values)");
}


