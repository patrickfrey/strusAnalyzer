/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#include "snowball.hpp"
#include "libstemmer.h"
#include "textwolf/charset_utf8.hpp"
#include "textwolf/cstringiterator.hpp"
#include "private/utils.hpp"
#include <stdexcept>

using namespace strus;

class StemNormalizer
	:public NormalizerInterface
{
public:
	enum DiaType {DiaTypeUnknown, DiaTypeGerman};

public:
	StemNormalizer(){}

	class ThisArgument
		:public NormalizerInterface::Argument
	{
	public:
		ThisArgument( const std::string& language)
		{
			std::string language_lo = utils::tolower( language);
			m_stemmer = sb_stemmer_new_threadsafe( language_lo.c_str(), 0/*UTF-8 is default*/);
			if (!m_stemmer)
			{
				throw std::runtime_error( std::string( "unconfigured language '") + language + "' passed to snowball stemmer");
			}
			if (language_lo == "de")
			{
				m_diatype = DiaTypeGerman;
			}
			else
			{
				m_diatype = DiaTypeUnknown;
			}
		}

		virtual ~ThisArgument()
		{
			if (m_stemmer)
			{
				sb_stemmer_delete( m_stemmer);
			}
		}

		struct sb_stemmer* m_stemmer;
		DiaType m_diatype;
	};

	class ThisContext
		:public NormalizerInterface::Context
	{
	public:
		explicit ThisContext( const ThisArgument* arg_)
			:m_stemmer(arg_->m_stemmer)
			,m_env(sb_stemmer_create_env( arg_->m_stemmer))
			,m_diatype(arg_->m_diatype){}

		virtual ~ThisContext()
		{
			sb_stemmer_delete_env( m_stemmer, m_env);
		}

		struct sb_stemmer* m_stemmer;
		struct SN_env* m_env;
		DiaType m_diatype;
	};

	virtual Argument* createArgument( const std::vector<std::string>& arg) const
	{
		if (arg.size() != 1)
		{
			throw std::runtime_error( "illegal number of arguments passed to snowball stemmer");
		}
		return new ThisArgument( arg[0]);
	}

	virtual Context* createContext( const Argument* arg) const
	{
		return new ThisContext( reinterpret_cast<const ThisArgument*>( arg));
	}

	static std::string substDiaCriticalToLower( ThisContext* ctx, const std::string& src)
	{
		std::string rt;
		textwolf::charset::UTF8 utf8;
		char buf[16];
		unsigned int bufpos;
		textwolf::CStringIterator itr( src.c_str(), src.size());

		while (*itr)
		{
			bufpos = 0;
			textwolf::UChar value = utf8.value( buf, bufpos, itr);
	
			if (value < 0x7F)
			{
				if (value >= 'A' && value <= 'Z')
				{
					rt.push_back( (char)value | 32);
				}
				else
				{
					rt.push_back( (char)value);
				}
			}
			else if (value <= 0xFF)
			{
				if ((value >= 0xC0 && value <= 0xC3) || value == 0xC5)
				{
					rt.push_back( 'a');
				}
				else if (value == 0xC6)
				{
					if (ctx->m_diatype == DiaTypeGerman)
					{
						rt.append( "ae");
					}
					else
					{
						rt.push_back( 'a');
					}
				}
				else if (value == 0xC4)
				{
					rt.append( "ae");
				}
				else if (value == 0xC7)
				{
					rt.push_back( 'c');
				}
				else if (value >= 0xC8 && value <= 0xCB)
				{
					rt.push_back( 'e');
				}
				else if (value >= 0xCC && value <= 0xCF)
				{
					rt.push_back( 'i');
				}
				else if (value == 0xD0)//CAPITAL ETH
				{
					rt.append( "th");
				}
				else if (value == 0xD1)
				{
					rt.push_back( 'n');
				}
				else if (value >= 0xD2 && value <= 0xD5)
				{
					rt.push_back( 'o');
				}
				else if (value == 0xD6)
				{
					if (ctx->m_diatype == DiaTypeGerman)
					{
						rt.append( "oe");
					}
					else
					{
						rt.push_back( 'o');
					}
				}
				else if (value == 0xD7)
				{
					rt.push_back( '*');
				}
				else if (value == 0xD8)
				{
					rt.push_back( 'o');
				}
				else if (value == 0xDC)
				{
					if (ctx->m_diatype == DiaTypeGerman)
					{
						rt.append( "ue");
					}
					else
					{
						rt.push_back( 'u');
					}
				}
				else if (value >= 0xD9 && value <= 0xDB)
				{
					rt.push_back( 'u');
				}
				else if (value == 0xDD)
				{
					rt.push_back( 'y');
				}
				else if (value == 0xDE)//CAPITAL THORN
				{
					rt.append( "th");
				}
				else if (value == 0xDF)
				{
					rt.append( "ss");
				}
				else if (value == 0xE4)
				{
					if (ctx->m_diatype == DiaTypeGerman)
					{
						rt.append( "ae");
					}
					else
					{
						rt.push_back( 'a');
					}
				}
				else if ((value >= 0xE0 && value <= 0xE3) || value == 0xE4)
				{
					rt.push_back( 'a');
				}
				else if (value == 0xE6)
				{
					rt.append( "ae");
				}
				else if (value == 0xE7)
				{
					rt.push_back( 'c');
				}
				else if (value >= 0xE8 && value <= 0xEB)
				{
					rt.push_back( 'e');
				}
				else if (value >= 0xEC && value <= 0xEF)
				{
					rt.push_back( 'i');
				}
				else if (value == 0xF0)//SMALL ETH
				{
					rt.append( "th");
				}
				else if (value == 0xF1)
				{
					rt.push_back( 'n');
				}
				else if (value >= 0xF2 && value <= 0xF5)
				{
					rt.push_back( 'o');
				}
				else if (value == 0xF6)
				{
					if (ctx->m_diatype == DiaTypeGerman)
					{
						rt.append( "oe");
					}
					else
					{
						rt.push_back( 'o');
					}
				}
				else if (value == 0xF7)
				{
					rt.push_back( '/');
				}
				else if (value == 0xF8)
				{
					rt.push_back( 'o');
				}
				else if (value == 0xFC)
				{
					if (ctx->m_diatype == DiaTypeGerman)
					{
						rt.append( "ue");
					}
					else
					{
						rt.push_back( 'u');
					}
				}
				else if (value >= 0xF9 && value <= 0xFB)
				{
					rt.push_back( 'u');
				}
				else if (value == 0xFD)
				{
					rt.push_back( 'y');
				}
				else if (value == 0xFE)
				{
					rt.append( "th");
				}
				else if (value == 0xFF)
				{
					rt.push_back( 'y');
				}
			}
			else if (value <= 0x2FF)
			{
				//romanian characters above 0xFF:
				if (value == 0x102)
				{
					rt.push_back( 'a');
				}
				else if (value == 0x103)
				{
					rt.push_back( 'a');
				}
				else if (value == 0x218 || value == 0x15F)
				{
					rt.push_back( 's');
				}
				else if (value == 0x219 || value == 0x15E)
				{
					rt.push_back( 's');
				}
				else if (value == 0x21A || value == 0x162)
				{
					rt.push_back( 't');
				}
				else if (value == 0x21B || value == 0x163)
				{
					rt.push_back( 't');
				}
			}
			else if (value == 0x1E9E)
			{
				rt.append( "ss");
			}
		}
		return rt;
	}

	virtual std::string normalize( Context* ctx_, const char* src, std::size_t srcsize) const
	{
		ThisContext* ctx = reinterpret_cast<ThisContext*>( ctx_);
		const sb_symbol* res
			= sb_stemmer_stem_threadsafe( ctx->m_stemmer, ctx->m_env, (const sb_symbol*)src, srcsize);
		if (!res) throw std::bad_alloc();
		std::size_t len = (std::size_t)sb_stemmer_length_threadsafe( ctx->m_env);

		return substDiaCriticalToLower( ctx, std::string( (const char*)res, len));
	}
};

const NormalizerInterface* strus::snowball_stemmer()
{
	static const StemNormalizer rt;
	return &rt;
}


