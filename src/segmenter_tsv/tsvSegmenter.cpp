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

TSVSegmenterContext::TSVSegmenterContext( const TSVParserDefinition& parserDefinition, const strus::Reference<strus::utils::TextEncoderBase>& decoder_, strus::ErrorBufferInterface *errbuf, bool errorReporting )
	: m_errorhnd( errbuf ), m_errorReporting( errorReporting), m_parser( ),
	m_parserDefinition( parserDefinition ), m_pos( -3 ),
	m_decoder(decoder_), m_eof(false), m_buf()
{}

TSVSegmenterContext::~TSVSegmenterContext( )
{
}

std::vector<std::string> TSVParser::splitLine( const std::string& line)
{
	std::vector<std::string> rt;
	char const* si = line.c_str();
	char const* sn = std::strchr( si, '\t');
	if (!*si) return std::vector<std::string>();

	for (; sn; sn = std::strchr( si=sn+1, '\t'))
	{
		rt.push_back( std::string( si, sn-si));
	}
	rt.push_back( si);
	return rt;
}

void TSVParser::parseHeader()
{
	std::string line;
	std::getline( m_is, line);
	if(m_is.eof())
	{
		m_eof = true;
		return;
	}
	m_header = splitLine( line);
}

bool TSVParser::nextLine()
{
	if (m_eof) return false;

	std::string line;
	std::getline( m_is, line);
	if(m_is.eof())
	{
		m_eof = true;
		return false;
	}
	m_data = splitLine( line);
	m_data.resize( m_header.size());
	++m_lineno;
	return true;
}

const char* TSVParser::linenostr()
{
	snprintf( m_linenostr, sizeof(m_linenostr)-1, "%d", lineno());
	return m_linenostr;
}

void TSVSegmenterContext::putInput( const char *chunk, std::size_t chunksize, bool eof )
{
	try
	{	
#ifdef STRUS_LOWLEVEL_DEBUG	
	std::cout << "DEBUG: putInput '" << chunksize << " (eof: " << eof << ")" << std::endl;
#endif
	
	if( m_eof ) {
		m_errorhnd->report( ErrorCodeOperationOrder, _TXT("fed chunk after declared end of input" ));
		return;
	}
	m_buf.append( chunk, chunksize );
	if (eof)
	{
		if (m_decoder.get())
		{
			m_buf = m_decoder->convert( m_buf.c_str(), m_buf.size(), true );
		}
		m_eof = true;
		m_parser.init( m_buf );
	}
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error in put input of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}


bool TSVSegmenterContext::parseData( int &id, strus::SegmenterPosition &pos, const char *&segment, std::size_t &segmentsize )
{
	// start of line, emit start of section
	segment = 0;
	segmentsize = 0;

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
			segment = m_parser.linenostr();
			segmentsize = strlen( segment );
			return true;
		}

		if( !m_parserDefinition.moreOftheSame( ) ) {
			m_pos++;
		}
	}

	while (m_pos < m_parser.cols()) {
		std::string headerName = m_parser.header( m_pos);
#ifdef STRUS_LOWLEVEL_DEBUG	
		std::cout << "DEBUG: field " << m_linepos << ":" << m_pos << "'" << headerName << "': '" << m_parser.col( m_pos) << "'" << std::endl;
#endif
		id = m_parserDefinition.getNextId( headerName );
		if( id == 0 ) {
			m_pos++;
		}
		else
		{
			break;
		}
		
	}
	if( id != 0 ) {
		// TODO: think whether we should not return the character position into the TSV here
		pos = m_parser.lineno() * m_parser.cols() + m_pos;
		segment = m_parser.col( m_pos).c_str( );
		segmentsize = m_parser.col( m_pos).size( );
		if( !m_parserDefinition.moreOftheSame( ) ) {
			m_pos++;
		}
		if(m_pos >= m_parser.cols()) {
			m_pos = -3;
			id = m_parserDefinition.getEndId( );
			if( id != 0 ) {
				return true;
			}
		}
		return true;
	}
	m_pos++;
	if( m_pos >= m_parser.cols() ) {
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
	for (;;)
	{
		if (m_parser.eof()) return false;
	
		if (m_pos < -2)
		{
			if (!m_parser.nextLine()) return false;
			m_pos = -2;
		}
		if ( parseData( id, pos, segment, segmentsize ) ) {
			return true;
		}
	}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in get next of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, false);
}

// TSVSegmenterInstance

TSVSegmenterInstance::TSVSegmenterInstance( strus::ErrorBufferInterface *errbuf, bool errorReporting )
	: m_errorhnd( errbuf ), m_errorReporting( errorReporting ), m_parserDefinition( )
{
}

void TSVSegmenterInstance::defineSelectorExpression( int id, const std::string &expression )
{
	try
	{
	m_parserDefinition.defineSelectorExpression( id, expression );
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error defining selector expression of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}
		
void TSVSegmenterInstance::defineSubSection( int startId, int endId, const std::string &expression )
{
	try
	{
		m_parserDefinition.defineSubSection( startId, endId, expression );
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error definint subsection of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}

strus::SegmenterContextInterface* TSVSegmenterInstance::createContext( const strus::analyzer::DocumentClass &dclass ) const
{
	try
	{
	strus::Reference<strus::utils::TextEncoderBase> decoder;
	if (!dclass.encoding().empty() && !strus::caseInsensitiveEquals( dclass.encoding(), "utf-8"))
	{
		decoder.reset( strus::utils::createTextDecoder( dclass.encoding().c_str()));
	}
	return new TSVSegmenterContext( m_parserDefinition, decoder, m_errorhnd, m_errorReporting );
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating context of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

strus::SegmenterMarkupContextInterface* TSVSegmenterInstance::createMarkupContext( const strus::analyzer::DocumentClass& dclass, const std::string& content) const
{
	try
	{
	m_errorhnd->report( ErrorCodeNotImplemented, _TXT("document markup not implemented for '%s' segmenter"), SEGMENTER_NAME);
	return 0;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating markup instance of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

strus::StructView TSVSegmenterInstance::view() const
{
	try
	{
		return strus::StructView()("name",name())
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, strus::StructView());
}

// TSVSegmenter

TSVSegmenter::TSVSegmenter( strus::ErrorBufferInterface *errbuf, bool errorReporting )
	: m_errorhnd( errbuf ), m_errorReporting( errorReporting )
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
	return new TSVSegmenterInstance( m_errorhnd, m_errorReporting );
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating instance of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

strus::ContentIteratorInterface* TSVSegmenter::createContentIterator(
		const char* content,
		std::size_t contentsize,
		const std::vector<std::string>& attributes,
		const std::vector<std::string>& expressions,
		const strus::analyzer::DocumentClass& dclass,
		const strus::analyzer::SegmenterOptions& opts) const
{
	try
	{
		if (!opts.items().empty()) throw strus::runtime_error(_TXT("no options defined for segmenter '%s'"), SEGMENTER_NAME);
		strus::Reference<strus::utils::TextEncoderBase> decoder;
		if (dclass.defined() && !strus::caseInsensitiveEquals( dclass.encoding(), "utf-8"))
		{
			decoder.reset( utils::createTextDecoder( dclass.encoding().c_str()));
		}
		return new TSVContentIterator( content, contentsize, attributes, expressions, decoder, m_errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating content iterator of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

strus::StructView TSVSegmenter::view() const
{
	try
	{
		return strus::StructView()
			("name", name())
			("mimetype", mimeType())
			("description", _TXT("Segmenter for TSV (text/tab-separated-values)"))
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, strus::StructView());
}

TSVContentIterator::TSVContentIterator( 
		const char* content_,
		std::size_t contentsize_,
		const std::vector<std::string>& attributes_,
		const std::vector<std::string>& expressions_,
		const strus::Reference<strus::utils::TextEncoderBase>& decoder_,
		ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_attributes(attributes_.begin(),attributes_.end()),m_content(),m_parser(),m_pos(-3),m_decoder(decoder_)
{
	try
	{
		if (m_decoder.get())
		{
			m_content = m_decoder->convert( content_, contentsize_, true);
		}
		else
		{
			m_content = std::string( content_, contentsize_);
		}
		m_parser.init( m_content);
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error fetching next element of content iterator of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}

bool TSVContentIterator::getNext(
		const char*& expression, std::size_t& expressionsize,
		const char*& segment, std::size_t& segmentsize)
{
	try
	{
		if (m_parser.eof()) return false;

		if (m_pos >= m_parser.cols())
		{
			m_pos = -3;
		}
		if (m_pos < 0)
		{
			if (!m_parser.nextLine()) return false;
			m_pos = 0;
			expression = "lineno";
			expressionsize = 6;
			segment = m_parser.linenostr();
			segmentsize = std::strlen( segment);
		}
		else
		{
			expression = m_parser.header( m_pos).c_str();
			expressionsize = m_parser.header( m_pos).size();
			segment = m_parser.col( m_pos).c_str();
			segmentsize = m_parser.col( m_pos).size();
			++m_pos;
		}
		return true;
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error fetching next element of content iterator of '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, false);
}



