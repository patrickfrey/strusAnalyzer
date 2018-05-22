/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Result format string printer
/// \file "libstrus_pattern_resultformat.cpp"
#include "strus/lib/pattern_resultformat.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "strus/base/utf8.hpp"
#include "strus/base/dll_tags.hpp"
#include "strus/errorCodes.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include <cstring>
#include <limits>
#include <list>
#include <map>

using namespace strus;

struct PatternResultFormatElement
{
	enum Op {Variable,String};
	Op op;
	union {
		const char* str;
		int idx;
	} value;
	const char* separator;
};

namespace strus
{
struct PatternResultFormat
{
	const PatternResultFormatElement* ar;	///< array of elements
	std::size_t arsize;			///< number of elements in array 'ar'
	std::size_t estimated_allocsize;	///< estimated maximum length of the result printed in bytes including 0-byte terminator
};
}//namespace

class Allocator
{
public:
	enum {VariableResultElementSize = 32};

	Allocator()
		:m_last_alloc_pos(0),m_last_alloc_align(0){}

	~Allocator()
	{
		std::list<MemBlock>::iterator mi = m_memblocks.begin(), me = m_memblocks.end();
		for (; mi != me; ++mi)
		{
			std::free( mi->base);
		}
	}

	enum {MinBlockSize=32*1024/*32K*/};

	void* alloc( std::size_t size, int align=1)
	{
		if (size >= (std::size_t)std::numeric_limits<int>::max()) return NULL;
		if (m_memblocks.empty() || m_memblocks.back().size - m_memblocks.back().pos < (int)size + align)
		{
			std::size_t mm = MinBlockSize;
			for (; mm <= size; mm *= 2){} //... cannot loop forever because of limits condition at start
			char* ptr = (char*)std::malloc( mm);
			if (!ptr) return NULL;
			m_memblocks.push_back( MemBlock( ptr, mm));
		}
		MemBlock& mb = m_memblocks.back();
		while ((mb.pos & (align -1)) != 0) ++mb.pos;
		char* rt = mb.base + mb.pos;
		m_last_alloc_pos = mb.pos;
		m_last_alloc_align = align;
		mb.pos += size;
		return rt;
	}

	void* realloc_last_alloc( std::size_t size)
	{
		MemBlock& mb = m_memblocks.back();
		if (mb.pos - m_last_alloc_pos > (int)size)
		{
			mb.pos = m_last_alloc_pos + size;
			return mb.base + m_last_alloc_pos;
		}
		else if (mb.pos - m_last_alloc_pos < (int)size)
		{
			if (mb.size - m_last_alloc_pos > (int)size)
			{
				mb.pos = m_last_alloc_pos + size;
				return mb.base + m_last_alloc_pos;
			}
			else
			{
				char* rt = (char*)alloc( size, m_last_alloc_align);
				if (!rt) return NULL;
				std::memcpy( rt, mb.base + m_last_alloc_pos, mb.pos - m_last_alloc_pos);
				return rt;
			}
		}
		else
		{
			return mb.base + m_last_alloc_pos;
		}
	}

	void resize_last_alloc( std::size_t size)
	{
		MemBlock& mb = m_memblocks.back();
		if (mb.pos - m_last_alloc_pos > (int)size)
		{
			mb.pos = m_last_alloc_pos + size;
		}
	}

private:
	Allocator( const Allocator&){}		//< non copyable
	void operator=( const Allocator&){}	//< non copyable

private:
	struct MemBlock
	{
		char* base;
		int pos;
		int size;

		MemBlock()
			:base(0),pos(0),size(0){}
		MemBlock( const MemBlock& o)
			:base(o.base),pos(o.pos),size(o.size){}
		MemBlock( char* base_, std::size_t size_)
			:base(base_),pos(0),size(size_){}
	};
	int m_last_alloc_pos;
	int m_last_alloc_align;
	std::list<MemBlock> m_memblocks;
};

struct PatternResultFormatContext::Impl
{
	Impl(){}
	~Impl(){}

	Allocator allocator;

private:
	Impl( const Impl&){}		//< non copyable
	void operator=( const Impl&){}	//< non copyable
};

DLL_PUBLIC PatternResultFormatContext::PatternResultFormatContext( ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_),m_debugtrace(0),m_impl(0)
{
	try
	{
		m_impl = new Impl();
		DebugTraceInterface* dbgi = m_errorhnd->debugTrace();
		if (dbgi) m_debugtrace = dbgi->createTraceContext( "pattern");
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error creating %s: %s"), "PatternResultFormatContext", *m_errorhnd);
}

DLL_PUBLIC PatternResultFormatContext::~PatternResultFormatContext()
{
	if (m_impl) delete m_impl;
	if (m_debugtrace) delete m_debugtrace;
}

struct PatternResultFormatTable::Impl
{
	Impl(){}
	~Impl(){}

	Allocator allocator;
	std::map<std::string,const char*> separatormap;

private:
	Impl( const Impl&){}		//< non copyable
	void operator=( const Impl&){}	//< non copyable
};

DLL_PUBLIC PatternResultFormatTable::PatternResultFormatTable( ErrorBufferInterface* errorhnd_, const PatternResultFormatVariableMap* variableMap_)
	:m_errorhnd(errorhnd_),m_variableMap(variableMap_),m_impl(0)
{
	try
	{
		m_impl = new Impl();
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error creating %s: %s"), "PatternResultFormatTable", *m_errorhnd);
}

DLL_PUBLIC PatternResultFormatTable::~PatternResultFormatTable()
{
	if (m_impl) delete m_impl;
}


static inline char* printNumber( char* ri, const char* re, int num)
{
	std::size_t len;
	if (re - ri < 8) return 0;
	len = strus::utf8encode( ri, num);
	if (!len) return 0;
	return ri + len;
}

static char* printResultItemReference( char* ri, const char* re, const analyzer::PatternMatcherResultItem& item)
{
	if (ri == re) return NULL;
	if (item.start_origseg() == item.end_origseg() && item.end_origpos() > item.start_origpos())
	{
		*ri++ = '\1';
		if (0==(ri = printNumber( ri, re, item.start_origseg()))) return NULL;
		if (0==(ri = printNumber( ri, re, item.start_origpos()))) return NULL;
		if (0==(ri = printNumber( ri, re, item.end_origpos() - item.start_origpos()))) return NULL;
	}
	else
	{
		*ri++ = '\2';
		if (0==(ri = printNumber( ri, re, item.start_origseg()))) return NULL;
		if (0==(ri = printNumber( ri, re, item.start_origpos()))) return NULL;
		if (0==(ri = printNumber( ri, re, item.end_origseg()))) return NULL;
		if (0==(ri = printNumber( ri, re, item.end_origpos()))) return NULL;
	}
	return ri;
}

static inline char* printValue( char* ri, const char* re, const char* value)
{
	char const* vi = value;
	for (; ri < re && *vi; ++ri,++vi) *ri = *vi;
	return ri < re ? ri : NULL;
}

DLL_PUBLIC const char* PatternResultFormatContext::map( const PatternResultFormat* fmt, std::size_t nofItems, const analyzer::PatternMatcherResultItem* items)
{
	if (!m_impl) return NULL;

	if (fmt->arsize == 0) return NULL;
	if (fmt->arsize == 1 && fmt->ar[0].op == PatternResultFormatElement::String) return fmt->ar[0].value.str;

	int mm = fmt->estimated_allocsize + (nofItems * Allocator::VariableResultElementSize) + 128/*thumb estimate*/;
	char* result = (char*)m_impl->allocator.alloc( mm);
	if (!result)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
		return NULL;
	}
	char* ri = result;
	const char* re = result + mm - 1;

	PatternResultFormatElement const* fi = fmt->ar;
	const PatternResultFormatElement* fe = fmt->ar + fmt->arsize;
	for (; fi!=fe; ++fi)
	{
		char* next_ri = ri;
		switch (fi->op)
		{
			case PatternResultFormatElement::String:
			{
				next_ri = printValue( ri, re, fi->value.str);
			}
			break;
			case PatternResultFormatElement::Variable:
			{
				int nn = 0;
				char* loop_ri = ri;
				for (std::size_t ii=0; ii<nofItems; ++ii)
				{
					const analyzer::PatternMatcherResultItem& item = items[ii];
					if (item.name() == fi->value.str)
					//... comparing char* is OK here, because variable names point to a uniqe memory location
					{
						if (nn++ > 0)
						{
							next_ri = printValue( loop_ri, re, fi->separator);
							if (!next_ri) break;
							loop_ri = next_ri;
						}
						if (item.value())
						{
							next_ri = printValue( loop_ri, re, item.value());
						}
						else
						{
							next_ri = printResultItemReference( loop_ri, re, items[ii]);
						}
						if (!next_ri) break;
						loop_ri = next_ri;
					}
				}
				break;
			}
		}
		if (!next_ri)
		{
			std::size_t rsize = ri - result;
			mm = rsize * (rsize/4) + 1024;
			result = (char*)m_impl->allocator.realloc_last_alloc( mm);
			if (!result)
			{
				m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
				return NULL;
			}
			ri = result + rsize;
			re = result + mm - 1;
			--fi; //... compensate increment of fi in for loop
			if (m_debugtrace) m_debugtrace->event( "realloc result format", "size=%d newsize=%d", (int)rsize, mm);
			continue;
		}
		ri = next_ri;
	}
	*ri++ = '\0';
	m_impl->allocator.resize_last_alloc( ri-result);
	return result;
}

struct _STDALIGN
{
	int val;
};

static int countNofElements( const char* src)
{
	int rt = 0;
	char const* si = src;
	while (*si)
	{
		const char* last_si = si;
		for (; *si && *si != '{'; ++si)
		{
			if (*si == '\\' && si[1] == '{')
			{
				si += 1;
			}
		}
		if (si > last_si)
		{
			rt += 1; //... const string
		}
		if (!*si) return rt;
		rt += 1; //... variable
		for (++si; *si && *si != '}'; ++si)
		{
			if (*si == '\\' || *si == '{') return -1;
		}
		if (!*si) return -1;
		++si;
	}
	return rt;
}

static const PatternResultFormatElement emptyPatternResultFormatElement()
{
	PatternResultFormatElement elem;
	elem.op = PatternResultFormatElement::String;
	elem.value.str = "";
	elem.separator = "";
	return elem;
}

DLL_PUBLIC const PatternResultFormat* PatternResultFormatTable::createResultFormat( const char* src)
{
	if (!m_impl) return NULL;

	int nofElements = countNofElements( src);
	if (nofElements < 0)
	{
		m_errorhnd->report( ErrorCodeSyntax, _TXT("failed to parse pattern result format string '%s'"), src);
		return NULL;
	}
	if (nofElements == 0)
	{
		static const PatternResultFormatElement elem = emptyPatternResultFormatElement();
		static const PatternResultFormat rt = {&elem,1,0};
		return &rt;
	}
	PatternResultFormat* rt = (PatternResultFormat*)m_impl->allocator.alloc( sizeof( PatternResultFormat) + (nofElements * sizeof(PatternResultFormatElement)), sizeof(_STDALIGN));
	if (!rt)
	{
		m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
		return NULL;
	}
	PatternResultFormatElement* ar = (PatternResultFormatElement*)(void*)(rt+1);
	rt->arsize = nofElements;
	rt->ar = ar;
	rt->estimated_allocsize = 0;
	std::memset( ar, 0, (nofElements * sizeof(PatternResultFormatElement)));
	int ei = 0, ee = nofElements;

	char const* si = src;
	while (*si)
	{
		std::string valuebuf;
		for (; *si && *si != '{'; ++si)
		{
			if (*si == '\\')
			{
				++si;
				if (!*si)
				{
					m_errorhnd->report( ErrorCodeSyntax, _TXT("unexpected end of string"));
					return NULL;
				}
				if (*si == 'n')
				{
					valuebuf.push_back( '\n');
				}
				else if (*si == 'b')
				{
					valuebuf.push_back( '\b');
				}
				else if (*si == 'r')
				{
					valuebuf.push_back( '\r');
				}
				else if (*si == 't')
				{
					valuebuf.push_back( '\t');
				}
				else
				{
					valuebuf.push_back( *si);
				}
			}
			else
			{
				valuebuf.push_back( *si);
			}
		}
		if (!valuebuf.empty())
		{
			// Put content element:
			if (ei == ee)
			{
				m_errorhnd->report( ErrorCodeRuntimeError, _TXT("internal: failed to parse format source '%s'"), src);
				return NULL;
			}
			char* value = (char*)m_impl->allocator.alloc( valuebuf.size() + 1);
			if (value == NULL)
			{
				m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
				return NULL;
			}
			PatternResultFormatElement& elem = ar[ ei++];
			elem.op = PatternResultFormatElement::String;
			elem.value.str = value;
			std::memcpy( value, valuebuf.c_str(), valuebuf.size());
			value[ valuebuf.size()] = '\0';
			elem.separator = NULL;
			rt->estimated_allocsize += valuebuf.size();
		}
		if (*si == '{')
		{
			std::string name;
			std::string separator;

			// Put variable:
			try
			{
				for (++si; *si != '}' && *si != '|'; ++si)
				{
					name.push_back( *si);
				}
				if (*si == '|')
				{
					for (++si; *si != '}'; ++si)
					{
						separator.push_back( *si);
					}
				}
			}
			catch (const std::bad_alloc&)
			{
				m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
				return NULL;
			}
			++si;
			const char* variable = m_variableMap->getVariable( name);
			if (!variable)
			{
				m_errorhnd->report( ErrorCodeUnknownIdentifier, _TXT("unknown variable '%s'"), name.c_str());
				return NULL;
			}
			if (ei == ee)
			{
				m_errorhnd->report( ErrorCodeRuntimeError, _TXT("internal: failed to parse format source '%s'"), src);
				return NULL;
			}
			PatternResultFormatElement& elem = ar[ ei++];
			elem.op = PatternResultFormatElement::Variable;
			elem.value.str = variable;
			if (separator.empty())
			{
				elem.separator = " ";
			}
			else
			{
				std::map<std::string,const char*>::const_iterator xi = m_impl->separatormap.find( separator);
				if (xi != m_impl->separatormap.end())
				{
					elem.separator = xi->second;
				}
				else
				{
					char* separatorptr = (char*)m_impl->allocator.alloc( separator.size()+1);
					if (!separatorptr)
					{
						m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
						return NULL;
					}
					std::memcpy( separatorptr, separator.c_str(), separator.size());
					separatorptr[ separator.size()] = '\0';
					elem.separator = separatorptr;
					try
					{
						m_impl->separatormap[ separator] = elem.separator;
					}
					catch (const std::bad_alloc&)
					{
						m_errorhnd->report( ErrorCodeOutOfMem, _TXT("out of memory"));
						return NULL;
					}
				}
			}
			rt->estimated_allocsize += Allocator::VariableResultElementSize;
		}
	}
	if (ei != ee)
	{
		m_errorhnd->report( ErrorCodeRuntimeError, _TXT("internal: error counting result size for expression '%s' (%d != %d)"), src, ei, ee);
		return NULL;
	}
	return rt;
}


static int decodeInt( char const*& src)
{
	if (*src == '\0') return 0;
	int chlen = strus::utf8charlen( *src);
	int rt = strus::utf8decode( src, chlen);
	src += chlen;
	return rt;
}

DLL_PUBLIC bool PatternResultFormatChunk::parseNext( PatternResultFormatChunk& result, char const*& src)
{
	std::memset( &result, 0, sizeof(PatternResultFormatChunk));
	const char* start = src;

	while ((unsigned char)*src > 2) ++src;
	if (start == src)
	{
		if (*src == '\0')
		{
			return false;
		}
		if (*src == '\1')
		{
			++src;
			result.start_seg = decodeInt( src);
			result.start_pos = decodeInt( src);
			int len = decodeInt( src);
			result.end_seg = result.start_seg;
			result.end_pos = result.start_pos + len;
		}
		else//if (*src == '\2')
		{
			++src;
			result.start_seg = decodeInt( src);
			result.start_pos = decodeInt( src);
			result.end_seg = decodeInt( src);
			result.end_pos = decodeInt( src);
		}
		return true;
	}
	else
	{
		result.value = start;
		result.valuesize = src - start;
		return true;
	}
}


std::string parsePatternResultFormatMapStringElement( char const*& si)
{
	std::string rt;
	int lv = 0;
	for (; *si; ++si)
	{
		if (lv == 0 && *si == '|')
		{
			++si;
			break;
		}
		else if (*si == '\\')
		{
			rt.push_back( *si);
			++si;
			if (!*si) throw std::runtime_error(_TXT("unexpected end of format string"));
			rt.push_back( *si);
		}
		else
		{
			if (*si == '{')
			{
				++lv;
			}
			else if (*si == '}')
			{
				--lv;
			}
			rt.push_back( *si);
		}
	}
	return rt;
}

static const char* g_patternMatcherResult_VariableMap_names[] = {"ordpos","ordlen","ordend","startseg","startpos","endseg","endpos","name","value",0};
enum PatternMatcherResult_Variable {VAR_ordpos,VAR_ordlen,VAR_ordend,VAR_startseg,VAR_startpos,VAR_endseg,VAR_endpos,VAR_name,VAR_value};

class PatternMatcherResult_VariableMap
	:public PatternResultFormatVariableMap
{
public:
	PatternMatcherResult_VariableMap(){}
	virtual ~PatternMatcherResult_VariableMap(){}

	virtual const char* getVariable( const std::string& name) const
	{
		char const* const* mi = g_patternMatcherResult_VariableMap_names;
		for (; *mi; ++mi) if (name == *mi) return *mi;
		return NULL;
	}

	void formatRewrite( const PatternResultFormat* fmt)
	{
		PatternResultFormatElement* ar = const_cast<PatternResultFormatElement*>( fmt->ar);
		int ai=0, ae=fmt->arsize;
		for (; ai != ae; ++ai)
		{
			char const* const* mi = g_patternMatcherResult_VariableMap_names;
			if (ar[ai].op == PatternResultFormatElement::Variable)
			{
				int midx = 0;
				for (; *mi; ++mi,++midx)
				{
					if (*mi == ar[ai].value.str)
					{
						ar[ai].value.idx = midx;
						break;
					}
				}
				if (!*mi)
				{
					throw std::runtime_error(_TXT("internal: unknown result item variable"));
				}
			}
		}
	}
};

static PatternMatcherResult_VariableMap g_patternResultFormatVariableMap;


struct PatternResultFormatMap::Impl
{
	PatternResultFormatTable table;
	const PatternResultFormat* fmt_result;
	const PatternResultFormat* fmt_resultItem;
	std::string sep_resultItem;

	Impl( ErrorBufferInterface* errorhnd_, const std::string& resultFmtStr, const std::string& resultItemFmtStr, const std::string& resultItemSep_)
		:table(errorhnd_,&g_patternResultFormatVariableMap),fmt_result(0),fmt_resultItem(0),sep_resultItem(resultItemSep_)
	{
		fmt_result = table.createResultFormat( resultFmtStr.c_str());
		fmt_resultItem = table.createResultFormat( resultItemFmtStr.c_str());

		if (!fmt_result || !fmt_resultItem) throw std::runtime_error( errorhnd_->fetchError());
		g_patternResultFormatVariableMap.formatRewrite( fmt_result);
		g_patternResultFormatVariableMap.formatRewrite( fmt_resultItem);
	}
};

DLL_PUBLIC PatternResultFormatMap::PatternResultFormatMap( ErrorBufferInterface* errorhnd_, const char* src_)
	:m_errorhnd(errorhnd_)
{
	try
	{
		char const* si = src_;
		std::string resultFormat = parsePatternResultFormatMapStringElement(si);
		std::string resultItemFormat = parsePatternResultFormatMapStringElement(si);
		std::string resultItemSeparator( si);

		m_impl = new Impl( errorhnd_, resultFormat, resultItemFormat, resultItemSeparator);
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error creating %s: %s"), "PatternResultFormatMap", *m_errorhnd);
}

DLL_PUBLIC PatternResultFormatMap::~PatternResultFormatMap()
{
	if (m_impl) delete m_impl;
}

static void printNumber( std::string& dest, int num)
{
	char numbuf[ 128];
	std::snprintf( numbuf, sizeof(numbuf), "%d", num);
	dest.append( numbuf);
}

std::string PatternResultFormatMap::mapItem( const analyzer::PatternMatcherResultItem& res) const
{
	std::string rt;

	PatternResultFormatElement const* ei = m_impl->fmt_resultItem->ar;
	const PatternResultFormatElement* ee = ei + m_impl->fmt_resultItem->arsize;
	for (; ei != ee; ++ei)
	{
		switch (ei->op)
		{
			case PatternResultFormatElement::String:
				rt.append( ei->value.str);
				break;
			case PatternResultFormatElement::Variable:
				switch ((PatternMatcherResult_Variable)ei->value.idx)
				{
					case VAR_ordpos:
						printNumber( rt, res.start_ordpos());
						break;
					case VAR_ordlen:
						printNumber( rt, res.end_ordpos() - res.start_ordpos());
						break;
					case VAR_ordend:
						printNumber( rt, res.end_ordpos());
						break;
					case VAR_startseg:
						printNumber( rt, res.start_origseg());
						break;
					case VAR_startpos:
						printNumber( rt, res.start_origpos());
						break;
					case VAR_endseg:
						printNumber( rt, res.end_origseg());
						break;
					case VAR_endpos:
						printNumber( rt, res.end_origpos());
						break;
					case VAR_name:
						rt.append( res.name());
						break;
					case VAR_value:
						if (res.value())
						{
							rt.append( res.value());
						}
						else
						{
							char buf[ 128];
							char* bi = buf;
							char* be = bi + sizeof(buf);
							bi = printResultItemReference( bi, be, res);
							rt.append( buf, bi-buf);
						}
						break;
					default:
						std::runtime_error(_TXT("data corruption in format list"));
						break;
				}
				break;
		}
	}
	return rt;
}

DLL_PUBLIC std::string PatternResultFormatMap::map( const analyzer::PatternMatcherResult& res) const
{
	try
	{
		if (!m_impl) return std::string();

		std::string rt;
		PatternResultFormatElement const* ei = m_impl->fmt_result->ar;
		const PatternResultFormatElement* ee = ei + m_impl->fmt_result->arsize;
		for (; ei != ee; ++ei)
		{
			switch (ei->op)
			{
				case PatternResultFormatElement::String:
					rt.append( ei->value.str);
					break;
				case PatternResultFormatElement::Variable:
					switch ((PatternMatcherResult_Variable)ei->value.idx)
					{
						case VAR_ordpos:
							printNumber( rt, res.start_ordpos());
							break;
						case VAR_ordlen:
							printNumber( rt, res.end_ordpos() - res.start_ordpos());
							break;
						case VAR_ordend:
							printNumber( rt, res.end_ordpos());
							break;
						case VAR_startseg:
							printNumber( rt, res.start_origseg());
							break;
						case VAR_startpos:
							printNumber( rt, res.start_origpos());
							break;
						case VAR_endseg:
							printNumber( rt, res.end_origseg());
							break;
						case VAR_endpos:
							printNumber( rt, res.end_origpos());
							break;
						case VAR_name:
							rt.append( res.name());
							break;
						case VAR_value:
						{
							std::vector<analyzer::PatternMatcherResultItem>::const_iterator
								ri = res.items().begin(), re = res.items().end();
							for (int ridx=0; ri != re; ++ri,++ridx)
							{
								if (ridx)
								{
									rt.append( m_impl->sep_resultItem);
								}
								rt.append( mapItem( *ri));
							}
							break;
						}
						default:
							std::runtime_error(_TXT("data corruption in format list"));
							break;
					}
					break;
			}
		}
		return rt;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error mapping pattern matcher result: %s"), *m_errorhnd, std::string());
}


