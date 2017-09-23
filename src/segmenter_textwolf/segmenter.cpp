/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "segmenter.hpp"
#include "segmenterContext.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "textwolf/charset.hpp"
#include "private/utils.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "segmenterMarkupContext.hpp"

using namespace strus;

#define SEGMENTER_NAME "textwolf"

void SegmenterInstance::defineSelectorExpression( int id, const std::string& expression)
{
	try
	{
		m_automaton.defineSelectorExpression( id, expression);
	}
	CATCH_ERROR_MAP_ARG1( _TXT("error defining expression for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}


void SegmenterInstance::defineSubSection( int startId, int endId, const std::string& expression)
{
	try
	{
		m_automaton.defineSubSection( startId, endId, expression);
	}
	CATCH_ERROR_MAP_ARG1( _TXT("error defining subsection for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}


SegmenterContextInterface* SegmenterInstance::createContext( const analyzer::DocumentClass& dclass) const
{
	try
	{
		typedef textwolf::charset::UTF8 UTF8;
		typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::BE> UTF16BE;
		typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::LE> UTF16LE;
		typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::BE> UCS2BE;
		typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::LE> UCS2LE;
		typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::BE> UCS4BE;
		typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::LE> UCS4LE;
		typedef textwolf::charset::IsoLatin IsoLatin;
		unsigned char codepage = 1;
	
		if (dclass.encoding().empty())
		{
			return new SegmenterContext<UTF8>( m_errorhnd, &m_automaton);
		}
		else
		{
			if (utils::caseInsensitiveStartsWith( dclass.encoding(), "IsoLatin")
			||  utils::caseInsensitiveStartsWith( dclass.encoding(), "ISO-8859"))
			{
				char const* cc = dclass.encoding().c_str() + 8;
				if (*cc == '-')
				{
					++cc;
					if (*cc >= '1' && *cc <= '9' && cc[1] == '\0')
					{
						codepage = *cc - '0';
					}
					else
					{
						throw strus::runtime_error( _TXT("parse error in character set encoding: '%s'"), dclass.encoding().c_str());
					}
				}
				return new SegmenterContext<IsoLatin>( m_errorhnd, &m_automaton, IsoLatin(codepage));
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UTF-8"))
			{
				return new SegmenterContext<UTF8>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UTF-16")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-16BE"))
			{
				return new SegmenterContext<UTF16BE>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UTF-16LE"))
			{
				return new SegmenterContext<UTF16LE>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-2")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UCS-2BE"))
			{
				return new SegmenterContext<UCS2BE>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-2LE"))
			{
				return new SegmenterContext<UCS2LE>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-4")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UCS-4BE")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-32")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-32BE"))
			{
				return new SegmenterContext<UCS4BE>( m_errorhnd, &m_automaton);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-4LE")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-32LE"))
			{
				return new SegmenterContext<UCS4LE>( m_errorhnd, &m_automaton);
			}
			else
			{
				throw strus::runtime_error( "%s",  _TXT("only support for UTF-8,UTF-16BE,UTF-16LE,UTF-32BE,UCS-4BE,UTF-32LE,UCS-4LE and ISO-8859 (code pages 1 to 9) as character set encoding"));
			}
		}
	}
	CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in '%s' segmenter creating context: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

SegmenterMarkupContextInterface* SegmenterInstance::createMarkupContext( const analyzer::DocumentClass& dclass, const std::string& content) const
{
	try
	{
		typedef textwolf::charset::UTF8 UTF8;
		typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::BE> UTF16BE;
		typedef textwolf::charset::UTF16<textwolf::charset::ByteOrder::LE> UTF16LE;
		typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::BE> UCS2BE;
		typedef textwolf::charset::UCS2<textwolf::charset::ByteOrder::LE> UCS2LE;
		typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::BE> UCS4BE;
		typedef textwolf::charset::UCS4<textwolf::charset::ByteOrder::LE> UCS4LE;
		typedef textwolf::charset::IsoLatin IsoLatin;
		unsigned char codepage = 1;
	
		if (dclass.encoding().empty())
		{
			return new SegmenterMarkupContext<UTF8>( m_errorhnd, content);
		}
		else
		{
			if (utils::caseInsensitiveStartsWith( dclass.encoding(), "IsoLatin")
			||  utils::caseInsensitiveStartsWith( dclass.encoding(), "ISO-8859"))
			{
				char const* cc = dclass.encoding().c_str() + 8;
				if (*cc == '-')
				{
					++cc;
					if (*cc >= '1' && *cc <= '9' && cc[1] == '\0')
					{
						codepage = *cc - '0';
					}
					else
					{
						throw strus::runtime_error( _TXT("parse error in character set encoding: '%s'"), dclass.encoding().c_str());
					}
				}
				return new SegmenterMarkupContext<IsoLatin>( m_errorhnd, content, IsoLatin(codepage));
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UTF-8"))
			{
				return new SegmenterMarkupContext<UTF8>( m_errorhnd, content);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UTF-16")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-16BE"))
			{
				return new SegmenterMarkupContext<UTF16BE>( m_errorhnd, content);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UTF-16LE"))
			{
				return new SegmenterMarkupContext<UTF16LE>( m_errorhnd, content);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-2")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UCS-2BE"))
			{
				return new SegmenterMarkupContext<UCS2BE>( m_errorhnd, content);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-2LE"))
			{
				return new SegmenterMarkupContext<UCS2LE>( m_errorhnd, content);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-4")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UCS-4BE")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-32")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-32BE"))
			{
				return new SegmenterMarkupContext<UCS4BE>( m_errorhnd, content);
			}
			else if (utils::caseInsensitiveEquals( dclass.encoding(), "UCS-4LE")
			||       utils::caseInsensitiveEquals( dclass.encoding(), "UTF-32LE"))
			{
				return new SegmenterMarkupContext<UCS4LE>( m_errorhnd, content);
			}
			else
			{
				throw strus::runtime_error( "%s",  _TXT("only support for UTF-8,UTF-16BE,UTF-16LE,UTF-32BE,UCS-4BE,UTF-32LE,UCS-4LE and ISO-8859 (code pages 1 to 9) as character set encoding"));
			}
		}
	}
	CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error creating markup context for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

SegmenterInstanceInterface* Segmenter::createInstance( const analyzer::SegmenterOptions& opts) const
{
	try
	{
		if (!opts.items().empty()) throw strus::runtime_error(_TXT("no options defined for segmenter '%s'"), SEGMENTER_NAME);
		return new SegmenterInstance( m_errorhnd);
	}
	CATCH_ERROR_MAP_ARG1_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

const char* Segmenter::getDescription() const
{
	return _TXT("Segmenter for XML (application/xml) based on the textwolf library");
}


