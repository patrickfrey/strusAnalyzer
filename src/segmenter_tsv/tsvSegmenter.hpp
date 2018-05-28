/*
 * Copyright (C) 2016 Andreas Baumann
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TSV_SEGMENTER_H_INLCUDED
#define TSV_SEGMENTER_H_INLCUDED

#include "strus/errorBufferInterface.hpp"
#include "strus/segmenterInterface.hpp"
#include "strus/segmenterInstanceInterface.hpp"
#include "strus/segmenterContextInterface.hpp"
#include "strus/segmenterMarkupContextInterface.hpp"
#include "strus/contentIteratorInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/analyzer/functionView.hpp"
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
		TSVParserDefinition( const TSVParserDefinition& o)
			:m_map(o.m_map),m_startId(o.m_startId),m_endId(o.m_endId)
		{
			m_it = m_map.end();
			m_end = m_map.end();
		}

		void defineSelectorExpression( int id, const std::string &expression );
		void defineSubSection( int startId, int endId, const std::string &expression );
		int getNextId( const std::string &definition );
		bool moreOftheSame( );
		int getStartId( ) const { return m_startId; }
		int getEndId( ) const { return m_endId; }
	
	private:
	
		std::multimap<std::string, int> m_map;
		std::multimap<std::string, int>::const_iterator m_it, m_end;
		int m_startId;
		int m_endId;

		void printDefinitions( );
		
};

class TSVParser
{
public:
	TSVParser() :m_is(),m_header(),m_data(),m_lineno(0),m_eof(true){}
	~TSVParser(){}

	void init( const std::string& content)	{m_is.str(content); m_eof=false; m_lineno=0; parseHeader();}
	bool eof() const			{return m_eof;}
	int cols() const			{return m_data.size();}
	const std::string& col( int id)		{return m_data[id];}
	const std::string& header( int id)	{return m_header[id];}
	bool nextLine();
	int lineno() const			{return m_lineno;}
	const char* linenostr();

private:
	void parseHeader();
	static std::vector<std::string> splitLine( const std::string& line);

private:
	std::istringstream m_is;
	std::vector<std::string> m_header;
	std::vector<std::string> m_data;
	int m_lineno;
	char m_linenostr[32];
	bool m_eof;
};

class TSVSegmenterContext : public strus::SegmenterContextInterface
{		
	public:
	
		TSVSegmenterContext( const TSVParserDefinition& parserDefinition, const strus::Reference<strus::utils::TextEncoderBase>& encoder_, strus::ErrorBufferInterface *errbuf, const bool errorReporting );

		virtual ~TSVSegmenterContext( );
		
		virtual void putInput( const char *chunk, std::size_t chunksize, bool eof );
		
		virtual bool getNext( int &id, strus::SegmenterPosition &pos, const char *&segment, std::size_t &segmentsize );
	
	private:
		strus::ErrorBufferInterface *m_errorhnd;
		bool m_errorReporting;
		TSVParser m_parser;
		TSVParserDefinition m_parserDefinition;
		int m_pos;
		strus::Reference<strus::utils::TextEncoderBase> m_encoder;
		bool m_eof;
		std::string m_buf;

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

		virtual strus::analyzer::FunctionView view() const;

	private:

		strus::ErrorBufferInterface *m_errorhnd;
		bool m_errorReporting;
		TSVParserDefinition m_parserDefinition;
};


class TSVContentIterator
	:public strus::ContentIteratorInterface
{
public:
	TSVContentIterator( 
			const char* content_,
			std::size_t contentsize_,
			const strus::Reference<strus::utils::TextEncoderBase>& encoder_,
			strus::ErrorBufferInterface* errorhnd_);

	virtual ~TSVContentIterator()
	{}

	virtual bool getNext(
			const char*& expression, std::size_t& expressionsize,
			const char*& segment, std::size_t& segmentsize);

private:
	void clear();

private:
	strus::ErrorBufferInterface* m_errorhnd;
	std::string m_content;
	TSVParser m_parser;
	int m_pos;
	strus::Reference<strus::utils::TextEncoderBase> m_encoder;
};


class TSVSegmenter : public strus::SegmenterInterface
{
	public:

		TSVSegmenter( strus::ErrorBufferInterface *errbuf, const bool errorReporting = true );
		
		virtual const char* mimeType( ) const;

		virtual strus::SegmenterInstanceInterface* createInstance( const strus::analyzer::SegmenterOptions& opts) const;

		virtual strus::ContentIteratorInterface* createContentIterator(
				const char* content,
				std::size_t contentsize,
				const strus::analyzer::DocumentClass& dclass,
				const strus::analyzer::SegmenterOptions& opts) const;

		virtual const char* getDescription() const;

	private:

		strus::ErrorBufferInterface *m_errorhnd;
		bool m_errorReporting;
};

#endif
