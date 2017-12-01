/*
 * Copyright (c) 2017 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Conversion of character set encodings to UTF-8
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include "textwolf/textscanner.hpp"
#include "textwolf/cstringiterator.hpp"
#include "private/textEncoder.hpp"
#include "private/internationalization.hpp"

using namespace strus;
using namespace strus::utils;

template <typename IOCharset>
class TextEncoder
	:public TextEncoderBase
{
public:
	explicit TextEncoder( const IOCharset& charset_=IOCharset())
		:m_charset(charset_)
		,m_itr(charset_)
		,m_eom()
		,m_output()
		,m_src(0)
		,m_srcsize(0)
		,m_srcend(false)
	{
		m_itr.setSource( textwolf::SrcIterator( m_src, m_srcsize, &m_eom));
	}

	virtual ~TextEncoder(){}
	virtual std::string convert( const char* src, std::size_t srcsize, bool eof)
	{
		std::string rt;
		m_src = src;
		m_srcend = eof;
		m_srcsize = srcsize;
		m_itr.setSource( textwolf::SrcIterator( m_src, m_srcsize, m_srcend?0:&m_eom));

		if (!m_srcend && setjmp(m_eom) != 0)
		{
			return rt;
		}
		textwolf::UChar ch;
		if ((ch = *m_itr) != 0)
		{
			++m_itr;
			m_output.print( ch, rt);
		}
		return rt;
	}

private:
	typedef textwolf::TextScanner<textwolf::SrcIterator,IOCharset> TextScanner;
	IOCharset m_charset;			///< character set encoding
	TextScanner m_itr;			///< iterator on input
	jmp_buf m_eom;				///< end of message trigger
	textwolf::charset::UTF8 m_output;	///< output
	const char* m_src;			///< pointer to current chunk parsed
	std::size_t m_srcsize;			///< size of the current chunk parsed in bytes
	bool m_srcend;				///< true if end of message is in current chunk parsed
};

static std::string parseEncoding( const char* src)
{
	std::string rt;
	char const* cc = src;
	for (; *cc; ++cc)
	{
		if ((unsigned char)*cc > 32 && *cc != '-')
		{
			rt.push_back( *cc | 32);
		}
	}
	return rt;
}

TextEncoderBase* utils::createTextEncoder( const char* encoding)
{
	if (!encoding)
	{
		return new TextEncoder<textwolf::charset::UTF8>();
	}
	else
	{
		std::string enc = parseEncoding( encoding);
		if ((enc.size() >= 8 && std::memcmp( enc.c_str(), "isolatin", 8)== 0))
		{
			const char* codepage = enc.c_str() + 8;
			if (std::strlen( codepage) > 1 || codepage[0] < '0' || codepage[0] > '9')
			{
				throw strus::runtime_error( _TXT("unknown iso-latin code page index"));
			}
			if (codepage[0] == '1')
			{
				return new TextEncoder<textwolf::charset::IsoLatin>();
			}
			else
			{
				return new TextEncoder<textwolf::charset::IsoLatin>( textwolf::charset::IsoLatin( codepage[0] - '0'));
			}
		}
		if ((enc.size() >= 7 && std::memcmp( enc.c_str(), "iso8859", 7) == 0))
		{
			const char* codepage = enc.c_str() + 7;
			if (std::strlen( codepage) > 1 || codepage[0] < '0' || codepage[0] > '9')
			{
				throw strus::runtime_error( _TXT("unknown iso-latin code page index"));
			}
			if (codepage[0] == '1')
			{
				return new TextEncoder<textwolf::charset::IsoLatin>();
			}
			else
			{
				return new TextEncoder<textwolf::charset::IsoLatin>( textwolf::charset::IsoLatin( codepage[0] - '0'));
			}
		}
		else if (enc.size() == 0 || enc == "utf8")
		{
			return new TextEncoder<textwolf::charset::UTF8>();
		}
		else if (enc == "utf16" || enc == "utf16be")
		{
			return new TextEncoder<textwolf::charset::UTF16BE>();
		}
		else if (enc == "utf16le")
		{
			return new TextEncoder<textwolf::charset::UTF16LE>();
		}
		else if (enc == "ucs2" || enc == "ucs2be")
		{
			return new TextEncoder<textwolf::charset::UCS2BE>();
		}
		else if (enc == "ucs2le")
		{
			return new TextEncoder<textwolf::charset::UCS2LE>();
		}
		else if (enc == "utf32" || enc == "ucs4" || enc == "utf32be" || enc == "ucs4be")
		{
			return new TextEncoder<textwolf::charset::UCS4BE>();
		}
		else if (enc == "utf32le" || enc == "ucs4le")
		{
			return new TextEncoder<textwolf::charset::UCS4LE>();
		}
		else
		{
			throw strus::runtime_error( _TXT("unknown character set encoding"));
		}
	}
}

const char* utils::detectBOM( const char* src, std::size_t srcsize, std::size_t& BOM_size)
{
	BOM_size = 0;
	if (srcsize < 4) return "utf-8";
	const unsigned char* bom = (const unsigned char*)src;
	if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {BOM_size = 3; return "utf-8";}
	if (bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xFE && bom[3] == 0xFF) {BOM_size = 4; return "utf-32be";}
	if (bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] == 0x00) {BOM_size = 4; return "utf-32le";}
	if (bom[0] == 0xFE && bom[1] == 0xFF) {BOM_size = 2; return "utf-16be";}
	if (bom[0] == 0xFF && bom[1] == 0xFE) {BOM_size = 2; return "utf-16le";}
	return 0;
}

const char* utils::detectCharsetEncoding( const char* src, std::size_t srcsize)
{
	char const* ci = src;
	const char* ce = src + srcsize;
	unsigned int zcnt = 0;
	unsigned int max_zcnt = 0;
	unsigned int mcnt[ 4] = {0,0,0,0};
	for (int cidx=0; ci != ce; ++ci,++cidx)
	{
		if (*ci == 0x00)
		{
			++zcnt;
			++mcnt[ cidx % 4];
		}
		else if (max_zcnt < zcnt)
		{
			max_zcnt = zcnt;
			zcnt = 0;
		}
	}
	if (max_zcnt == 0)
	{
		return "utf-8";
	}
	if (mcnt[0] >= mcnt[1] && mcnt[1] >= mcnt[2] && mcnt[2] >= mcnt[3] && mcnt[3] == 0)
	{
		return "utf-32be";
	}
	if (mcnt[0] == 0 && mcnt[0] <= mcnt[1] && mcnt[1] <= mcnt[2] && mcnt[2] <= mcnt[3])
	{
		return "utf-32le";
	}
	if (mcnt[0] >= mcnt[1] && mcnt[2] >= mcnt[3] && mcnt[1] == 0 && mcnt[3] == 0)
	{
		return "utf-16be";
	}
	if (mcnt[0] == 0 && mcnt[2] == 0 && mcnt[0] <= mcnt[1] && mcnt[2] <= mcnt[3])
	{
		return "utf-16le";
	}
	return 0;
}

