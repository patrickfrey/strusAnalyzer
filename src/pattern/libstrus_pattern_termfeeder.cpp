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
			if (!id) throw strus::runtime_error(_TXT("used 0 as lexem identifier"));

			m_typeTable[ utils::tolower(type)] = id;
		}
		CATCH_ERROR_MAP( _TXT("cannot define term feeder lexem: %s"), *m_errorhnd);
	}

	virtual void defineSymbol(
			unsigned int id,
			unsigned int lexemid,
			const std::string& name)
	{
		try
		{
			if (!id) throw strus::runtime_error(_TXT("used 0 as symbol identifier"));

			std::size_t idx = m_symbolTable.getOrCreate( name);
			if (idx == 0) throw strus::runtime_error( m_errorhnd->fetchError());
			if (m_symbolTable.isNew())
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
		CATCH_ERROR_MAP( _TXT("cannot define term feeder symbol: %s"), *m_errorhnd);
	}

	virtual unsigned int getLexem(
			const std::string& type) const
	{
		TypeTable::const_iterator ti = m_typeTable.find( type);
		if (ti == m_typeTable.end()) return 0;
		return ti->second;
	}

	virtual std::vector<std::string> lexemTypes() const
	{
		std::vector<std::string> rt;
		TypeTable::const_iterator ti = m_typeTable.begin(), te = m_typeTable.end();
		for (; ti != te; ++ti)
		{
			rt.push_back( ti->first);
		}
		return rt;
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

private:
	typedef std::map<std::string,unsigned int> TypeTable;

private:
	ErrorBufferInterface* m_errorhnd;
	TypeTable m_typeTable;
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


