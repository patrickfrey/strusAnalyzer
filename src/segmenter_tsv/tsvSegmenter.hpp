/*
 * Copyright (C) 2016 Andreas Baumann
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TSV_SEGMENTER_H_INLCUDED
#define TSV_SEGMENTER_H_INLCUDED

#include "boost/scoped_ptr.hpp"

#include "strus/errorBufferInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/segmenterMarkupContextInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/reference.hpp"
#include "private/textEncoder.hpp"

#include <string> 
#include <vector> 
#include <map>
#include <stdexcept>
#include <sstream>

class TSVParserException : public std::runtime_error {
	
	public:
	
		TSVParserException( const std::string &what_ )
			: std::runtime_error( what_ ) { }
};

class TSVParserDefinition
{
	public:
	
		TSVParserDefinition( );
		
		void defineSelectorExpression( int id, const std::string &expression );
		void defineSubSection( int startId, int endId, const std::string &expression );
		int getNextId( const std::string &definition );
		bool moreOftheSame( );
		int getStartId( ) { return m_startId; }
		int getEndId( ) { return m_endId; }
	
	private:
	
		std::multimap<std::string, int> m_map;
		std::multimap<std::string, int>::const_iterator m_it, m_end;
		int m_startId;
		int m_endId;

		void printDefinitions( );
		
};

class TSVSegmenterContext : public strus::SegmenterContextInterface
{		
	public:
	
		TSVSegmenterContext( TSVParserDefinition *parserDefinition, const strus::Reference<strus::utils::TextEncoderBase>& encoder_, strus::ErrorBufferInterface *errbuf, const bool errorReporting );

		virtual ~TSVSegmenterContext( );
		
		virtual void putInput( const char *chunk, std::size_t chunksize, bool eof );
		
		virtual bool getNext( int &id, strus::SegmenterPosition &pos, const char *&segment, std::size_t &segmentsize );
	
	private:
	
		enum TSVParseState {
			TSV_PARSE_STATE_HEADER,
			TSV_PARSE_STATE_DATA,
			TSV_PARSE_STATE_EOF
		};
	
		strus::ErrorBufferInterface *m_errbuf;
		bool m_errorReporting;
		std::string m_buf;
		bool m_eof;
		std::istringstream m_is;
		std::string m_currentLine;
		TSVParserDefinition *m_parserDefinition;
		std::vector<std::string> m_data;
		int m_pos;
		unsigned int m_linepos;
		char m_lineposStr[12];
		enum TSVParseState m_parseState;
		std::vector<std::string> m_headers;
		strus::Reference<strus::utils::TextEncoderBase> m_encoder;

		bool parseHeader( );
		bool parseData( int &id, strus::SegmenterPosition &pos, const char *&segment, std::size_t &segmentsize );
};


class TSVSegmenterInstance : public strus::SegmenterInstanceInterface
{
	public:

		TSVSegmenterInstance( strus::ErrorBufferInterface *errbuf, const bool errorReporting );
		
		virtual void defineSelectorExpression( int id, const std::string &expression );
		
		virtual void defineSubSection( int startId, int endId, const std::string &expression );

		virtual strus::SegmenterContextInterface* createContext( const strus::analyzer::DocumentClass &dclass) const;
		virtual strus::SegmenterMarkupContextInterface* createMarkupContext( const strus::analyzer::DocumentClass& dclass, const std::string& content) const;

	private:

		strus::ErrorBufferInterface *m_errbuf;
		bool m_errorReporting;
		boost::scoped_ptr<TSVParserDefinition> m_parserDefinition;
};

class TSVSegmenter : public strus::SegmenterInterface
{
	public:

		TSVSegmenter( strus::ErrorBufferInterface *errbuf, const bool errorReporting = true );
		
		virtual const char* mimeType( ) const;

		virtual strus::SegmenterInstanceInterface* createInstance( const strus::analyzer::SegmenterOptions& opts) const;

		virtual const char* getDescription() const;

	private:

		strus::ErrorBufferInterface *m_errbuf;
		bool m_errorReporting;
};

#endif
