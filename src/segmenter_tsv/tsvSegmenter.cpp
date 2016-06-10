/*
 * Copyright (C) 2016 Andreas Baumann
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tsvSegmenter.hpp"

#undef LOWLEVEL_DEBUG

#ifdef LOWLEVEL_DEBUG
#include <iostream>
#endif

// TSVParserDefinition

TSVParserDefinition::TSVParserDefinition( )
	: m_map( ), m_it( m_map.end( ) ), m_end( m_map.end( ) )
{
}

void TSVParserDefinition::addDefinition( int id, const std::string &definition )
{
#ifdef LOWLEVEL_DEBUG	
	std::cout << "DEBUG: adding selector expression: " << id << ", " << definition << std::endl;
#endif
	m_map.insert( std::make_pair( definition, id ) );

#ifdef LOWLEVEL_DEBUG	
	std::cout << "DEBUG: definition multimap contains: ";
	for( std::multimap<std::string, int>::const_iterator it = m_map.begin( ); it != m_map.end( ); it++ ) {
		std::cout << "[" << it->first << ", " << it->second << "], ";
	}
	std::cout << std::endl;
#endif
}

int TSVParserDefinition::getNextId( const std::string &definition )
{
#ifdef LOWLEVEL_DEBUG	
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
		if( ret.first->first == definition ) {
			m_it = ret.first;
			m_end = ret.second;
			id = m_it->second;
		} else {
			m_it = m_map.end( );
			m_end = m_map.end( );
		}
	}
#ifdef LOWLEVEL_DEBUG	
	std::cout << "DEBUG: got number for '" << definition << "' to be " << id << std::endl;
#endif
	return id;
}

bool TSVParserDefinition::moreOftheSame( )
{
#ifdef LOWLEVEL_DEBUG	
	std::cout << "DEBUG: moreOftheSame is " <<  ( m_it != m_map.end( ) && m_it != m_end ) << std::endl;
#endif
	return m_it != m_map.end( ) && m_it != m_end;
}

// TSVSegmenterContext

TSVSegmenterContext::TSVSegmenterContext( TSVParserDefinition *parserDefinition, strus::ErrorBufferInterface *errbuf, bool errorReporting )
	: m_errbuf( errbuf ), m_errorReporting( errorReporting),
	m_eof( false ), m_parserDefinition( parserDefinition ),
	m_pos( 0 ),
	m_parseState( TSV_PARSE_STATE_HEADER ),
	m_is( m_buf )
{
}
	
TSVSegmenterContext::~TSVSegmenterContext( )
{
}
			
void TSVSegmenterContext::putInput( const char *chunk, std::size_t chunksize, bool eof )
{
	if( m_eof ) {
		m_errbuf->report( "fed chunk after declared end of input" );
		return;
	}
	
	m_buf.append( chunk, chunksize );
	m_eof = eof;
	
	if( !m_eof ) {
		return;
	}
	
	m_is.str( m_buf );
	m_is.clear( );
	std::istreambuf_iterator<char> begin( m_is );
	std::istreambuf_iterator<char> end;
}

bool TSVSegmenterContext::parseHeader( int &id, strus::SegmenterPosition &pos, const char *&segment, std::size_t &segmentsize )
{
	//~ if( m_hbit != m_heit ) {
		//~ do {
//~ #ifdef LOWLEVEL_DEBUG	
			//~ std::cout << "DEBUG: Header data: " << m_hbit->name( ) << ": " << m_hbit->value( ) << std::endl;
//~ #endif
			//~ std::string header = m_hbit->name( );
			//~ boost::algorithm::to_lower( header );
			//~ if( header == "content-transfer-encoding" ) {
				//~ transferEncoding = hbit->value( );
				//~ boost::algorithm::to_lower( transferEncoding );
			//~ }
			//~ id = m_parserDefinition->getNextId( header );
			//~ if( id == 0 && !m_parserDefinition->moreOftheSame( ) ) {
				//~ m_hbit++;
				//~ if( m_hbit != m_heit ) {
					//~ m_pos += header.size( ) + m_hbit->value( ).size( );
				//~ }
			//~ }
		//~ } while( id == 0 && m_hbit != m_heit );
		
		//~ if( m_hbit == m_heit ) {
//~ #ifdef LOWLEVEL_DEBUG	
			//~ std::cout << "DEBUG: no more header data" << std::endl;
//~ #endif
			//~ return false;
		//~ }
		
		//~ if( id != 0 ) {
			//~ pos = m_pos; // TODO: good idea for position to orginal source needed?
			//~ m_currentValue = m_hbit->value( );
			//~ segment = m_currentValue.c_str( );
			//~ segmentsize = m_currentValue.size( );
			//~ if( !m_parserDefinition->moreOftheSame( ) ) {
				//~ m_hbit++;
				//~ m_pos += segmentsize;
			//~ }
			//~ return true;
		//~ }
	//~ }
	
	return false;
}		

// TODO: state machine iterating through mime body parts (main, then parts( ) )
bool TSVSegmenterContext::parseBody( int &id, strus::SegmenterPosition &pos, const char *&segment, std::size_t &segmentsize )
{
#ifdef LOWLEVEL_DEBUG	
	std::cout << "DEBUG: TSV body" << std::endl;
#endif

	m_currentValue.clear( );

	// process my own content (recursively)
	//~ processMimeEntity( m_me.get( ), id, pos, segment, segmentsize );
// TODO: state machine iterating through mime body parts (main, then parts( ) )
	//~ mimetic::MimeEntityList::iterator mbit = m_me->body( ).parts( ).begin( ),
		//~ mend = m_me->body( ).parts( ).end( );
	//~ for( ; mbit != mend; mbit++ ) {
		//~ m_currentValue += " ";
		//~ processMimeEntity( *mbit, id, pos, segment, segmentsize );
	//~ }

	//~ id = m_parserDefinition->getNextId( "body" );
	//~ if( id != 0 ) {
		//~ pos = m_pos; // TODO: good idea for position to orginal source needed?
		//~ segment = m_currentValue.c_str( );
		//~ segmentsize = m_currentValue.size( );
		//~ if( !m_parserDefinition->moreOftheSame( ) ) {
			//~ m_parseState = TSV_PARSE_STATE_EOF;
		//~ }
		//~ return true;
	//~ }
		
	return false;
}

bool TSVSegmenterContext::getNext( int &id, strus::SegmenterPosition &pos, const char *&segment, std::size_t &segmentsize )
{
	if( !m_eof ) {
		return false;
	}

	bool hasNext = false;
	switch( m_parseState ) {
		case TSV_PARSE_STATE_HEADER:
			if( ( hasNext = parseHeader( id, pos, segment, segmentsize ) ) ) {
				return hasNext;
			} else {
				m_parseState = TSV_PARSE_STATE_BODY;
			}
			// intentional fall-thru here!
			
		case TSV_PARSE_STATE_BODY:
			return parseBody( id, pos, segment, segmentsize );

		case TSV_PARSE_STATE_EOF:
#ifdef LOWLEVEL_DEBUG
			std::cout << "DEBUG: end of data" << std::endl;
#endif
			return false;

	}
	
	return false;
}

// TSVSegmenterInstance

TSVSegmenterInstance::TSVSegmenterInstance( strus::ErrorBufferInterface *errbuf, bool errorReporting )
	: m_errbuf( errbuf ), m_errorReporting( errorReporting )
{
	m_parserDefinition.reset( new TSVParserDefinition( ) );
}

void TSVSegmenterInstance::defineSelectorExpression( int id, const std::string &expression )
{
	m_parserDefinition->addDefinition( id, expression );
}
		
void TSVSegmenterInstance::defineSubSection( int startId, int endId, const std::string &expression )
{
	// TODO
#ifdef LOWLEVEL_DEBUG	
	std::cout << "DEBUG: selector subsection: " << startId << ", " 
		<< endId << ", " << expression << std::endl;
#endif
}

strus::SegmenterContextInterface* TSVSegmenterInstance::createContext( const strus::DocumentClass &dclass ) const
{
	return new TSVSegmenterContext( m_parserDefinition.get( ), m_errbuf, m_errorReporting );
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

strus::SegmenterInstanceInterface* TSVSegmenter::createInstance( ) const
{
	return new TSVSegmenterInstance( m_errbuf, m_errorReporting );
}
