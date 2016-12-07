/*
* Copyright (c) 2014 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Library with the default implentation of the pattern term feeder interface
/// \file "libstrus_pattern_termfeeder.cpp"
#include "strus/lib/pattern_termfeeder.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/patternTermFeederInterface.hpp"
#include "strus/patternTermFeederInstanceInterface.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/base/symbolTable.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "private/utils.hpp"
#include <memory>
#include <string>
#include <vector>

using namespace strus;

static bool g_intl_initialized = false;

class PatternTermFeederInstance
	:public PatternTermFeederInstanceInterface
{
public:
	explicit PatternTermFeederInstance( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_),m_typeTable(),m_symbolTable(){}

	virtual ~PatternTermFeederInstance(){}

	virtual void defineLexem(
			unsigned int id,
			const std::string& type)
	{
		try
		{
			m_typeTable[ type] = id;
		}
		CATCH_ERROR_MAP( _TXT("cannot define term feeded lexem: %s"), *m_errorhnd);
	}

	virtual void defineSymbol(
			unsigned int id,
			unsigned int lexemid,
			const std::string& name)
	{
		try
		{
			std::size_t idx = m_symbolTable.getOrCreate( name);
			if (m_symbolTable.isNew(idx))
			{
				if (idx != m_syminfotab.size() +1) throw strus::runtime_error(_TXT("corrupt symbol table"));
				m_syminfoar.push_back( SymbolInfo( lexemid, id, 0));
				m_syminfotab.push_back( m_syminfoar.size());
			}
			else
			{
				std::size_t symidx = m_syminfotab[ idx-1];
				m_syminfoar.push_back( SymbolInfo( lexemid, id, symidx));
				m_syminfotab[ idx-1] = m_syminfoar.size();
			}
		}
		CATCH_ERROR_MAP( _TXT("cannot define term feeded lexem: %s"), *m_errorhnd);
	}

	virtual unsigned int getSymbol(
			unsigned int lexemid,
			const std::string& name) const
	{
		try
		{
			std::size_t idx = m_symbolTable.get( name);
			if (!idx) return 0;
			const SymbolInfo* itr = &m_syminfoar[ m_syminfotab[ idx-1] -1];
			for (;;)
			{
				if (itr->lexemid == lexemid) return itr->id;
				if (itr->next == 0) return 0;
				itr = &m_syminfoar[ itr->next -1];
			}
		}
		CATCH_ERROR_MAP_RETURN( _TXT("failed to retrieve lexem symbol: %s"), *m_errorhnd, 0);
	}

	virtual std::vector<analyzer::PatternLexem> mapTerms(
			const std::vector<analyzer::Term>& termlist) const
	{
		try
		{
			std::vector<analyzer::PatternLexem> rt;
			std::vector<analyzer::Term>::const_iterator ti = termlist.begin(), te = termlist.end();
			for (std::size_t tidx=0; ti != te; ++ti,++tidx)
			{
				std::map<std::string,unsigned int>::const_iterator pi = m_typeTable.find( ti->type());
				if (pi != m_typeTable.end())
				{
					unsigned int elemid = pi->second;
	
					std::size_t symidx = m_symbolTable.get( ti->value());
					if (symidx)
					{
						const SymbolInfo* itr = &m_syminfoar[ symidx -1];
						for (;;)
						{
							if (itr->lexemid == elemid)
							{
								elemid = itr->id;
								break;
							}
							if (itr->next == 0)
							{
								break;
							}
							itr = &m_syminfoar[ itr->next -1];
						}
					}
					rt.push_back( analyzer::PatternLexem( elemid, ti->pos(), 0, tidx, 1));
				}
			}
			return rt;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("failed to map analyzer term list to pattern matching lexems: %s"), *m_errorhnd, std::vector<analyzer::PatternLexem>());
	}

	virtual std::vector<analyzer::Term> mapResults(
			const std::string& resultFeatureType,
			const std::vector<analyzer::PatternMatcherResult>& resultList,
			const std::vector<analyzer::Term>& orig_termlist) const
	{
		try
		{
			std::vector<analyzer::Term> rt;
			std::vector<analyzer::PatternMatcherResult>::const_iterator
				ri = resultList.begin(), re = resultList.end();
			for (; ri != re; ++ri)
			{
				if (!resultFeatureType.empty())
				{
					rt.push_back( analyzer::Term( resultFeatureType, ri->name(), ri->ordpos()));
				}
				std::vector<analyzer::PatternMatcherResultItem>::const_iterator
					pi = ri->items().begin(), pe = ri->items().end();
				for (; pi != pe; ++pi)
				{
					std::size_t ii = pi->start_origpos(), ie = pi->end_origpos();
					if (ii+1 < ie)
					{
						std::string value;
						value.append( orig_termlist[ ii].value());
						for (++ii; ii < ie; ++ii)
						{
							value.push_back( ' ');
							value.append( orig_termlist[ ii].value());
						}
						rt.push_back( analyzer::Term( pi->name(), value, pi->ordpos()));
					}
					else
					{
						rt.push_back( analyzer::Term( pi->name(), orig_termlist[ pi->start_origpos()].value(), pi->ordpos()));
					}
				}
			}
			return rt;
		}
		CATCH_ERROR_MAP_RETURN( _TXT("failed to pattern matching results to analyzer term list: %s"), *m_errorhnd, std::vector<analyzer::Term>());
	}

private:
	ErrorBufferInterface* m_errorhnd;
	std::map<std::string,unsigned int> m_typeTable;
	SymbolTable m_symbolTable;

	struct SymbolInfo
	{
		unsigned int lexemid;
		unsigned int id;
		unsigned int next;

		SymbolInfo( unsigned int lexemid_, unsigned int id_, unsigned int next_)
			:lexemid(lexemid_),id(id_),next(next_){}
		SymbolInfo( const SymbolInfo& o)
			:lexemid(o.lexemid),id(o.id),next(o.next){}
	};
	std::vector<SymbolInfo> m_syminfoar;
	std::vector<std::size_t> m_syminfotab;
};


class PatternTermFeeder
	:public PatternTermFeederInterface
{
public:
	explicit PatternTermFeeder( ErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}
	virtual ~PatternTermFeeder(){}

	virtual PatternTermFeederInstanceInterface* createInstance() const
	{
		try
		{
			return new PatternTermFeederInstance( m_errorhnd);
		}
		CATCH_ERROR_MAP_RETURN( _TXT("failed to create pattern term feeder instance: %s"), *m_errorhnd, 0);
	}

private:
	ErrorBufferInterface* m_errorhnd;
};


DLL_PUBLIC PatternTermFeederInterface* strus::createPatternTermFeeder_default( ErrorBufferInterface* errorhnd)
{
	try
	{
		if (!g_intl_initialized)
		{
			strus::initMessageTextDomain();
			g_intl_initialized = true;
		}
		return new PatternTermFeeder( errorhnd);
	}
	CATCH_ERROR_MAP_RETURN( _TXT("cannot create pattern term feeder: %s"), *errorhnd, 0);
}


