/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "segmenter.hpp"
#include "segmenterContext.hpp"
#include "contentIterator.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/base/string_conv.hpp"
#include "textwolf/charset.hpp"
#include "private/errorUtils.hpp"
#include "private/internationalization.hpp"
#include "segmenterMarkupContext.hpp"

using namespace strus;

#define SEGMENTER_NAME "textwolf"

StructView TextwolfSegmenterInstance::view() const
{
	try
	{
		return StructView()
			("name", name())
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
}

void TextwolfSegmenterInstance::defineSelectorExpression( int id, const std::string& expression)
{
	try
	{
		if (expression.empty())
		{
			m_automaton.defineSelectorExpression( id, "//()");
		}
		else
		{
			m_automaton.defineSelectorExpression( id, expression);
		}
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error defining expression for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}

void TextwolfSegmenterInstance::defineSubSection( int startId, int endId, const std::string& expression)
{
	try
	{
		m_automaton.defineSubSection( startId, endId, expression);
	}
	CATCH_ERROR_ARG1_MAP( _TXT("error defining subsection for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd);
}


SegmenterContextInterface* TextwolfSegmenterInstance::createContext( const analyzer::DocumentClass& dclass) const
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
			if (strus::caseInsensitiveStartsWith( dclass.encoding(), "IsoLatin")
			||  strus::caseInsensitiveStartsWith( dclass.encoding(), "ISO-8859"))
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
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UTF-8"))
			{
				return new SegmenterContext<UTF8>( m_errorhnd, &m_automaton);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UTF-16")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-16BE"))
			{
				return new SegmenterContext<UTF16BE>( m_errorhnd, &m_automaton);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UTF-16LE"))
			{
				return new SegmenterContext<UTF16LE>( m_errorhnd, &m_automaton);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-2")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UCS-2BE"))
			{
				return new SegmenterContext<UCS2BE>( m_errorhnd, &m_automaton);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-2LE"))
			{
				return new SegmenterContext<UCS2LE>( m_errorhnd, &m_automaton);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-4")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UCS-4BE")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-32")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-32BE"))
			{
				return new SegmenterContext<UCS4BE>( m_errorhnd, &m_automaton);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-4LE")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-32LE"))
			{
				return new SegmenterContext<UCS4LE>( m_errorhnd, &m_automaton);
			}
			else
			{
				throw strus::runtime_error( "%s",  _TXT("only support for UTF-8,UTF-16BE,UTF-16LE,UTF-32BE,UCS-4BE,UTF-32LE,UCS-4LE and ISO-8859 (code pages 1 to 9) as character set encoding"));
			}
		}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' segmenter creating context: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

SegmenterMarkupContextInterface* TextwolfSegmenterInstance::createMarkupContext( const analyzer::DocumentClass& dclass, const std::string& content) const
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
			if (strus::caseInsensitiveStartsWith( dclass.encoding(), "IsoLatin")
			||  strus::caseInsensitiveStartsWith( dclass.encoding(), "ISO-8859"))
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
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UTF-8"))
			{
				return new SegmenterMarkupContext<UTF8>( m_errorhnd, content);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UTF-16")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-16BE"))
			{
				return new SegmenterMarkupContext<UTF16BE>( m_errorhnd, content);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UTF-16LE"))
			{
				return new SegmenterMarkupContext<UTF16LE>( m_errorhnd, content);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-2")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UCS-2BE"))
			{
				return new SegmenterMarkupContext<UCS2BE>( m_errorhnd, content);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-2LE"))
			{
				return new SegmenterMarkupContext<UCS2LE>( m_errorhnd, content);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-4")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UCS-4BE")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-32")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-32BE"))
			{
				return new SegmenterMarkupContext<UCS4BE>( m_errorhnd, content);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-4LE")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-32LE"))
			{
				return new SegmenterMarkupContext<UCS4LE>( m_errorhnd, content);
			}
			else
			{
				throw strus::runtime_error( "%s",  _TXT("only support for UTF-8,UTF-16BE,UTF-16LE,UTF-32BE,UCS-4BE,UTF-32LE,UCS-4LE and ISO-8859 (code pages 1 to 9) as character set encoding"));
			}
		}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error creating markup context for '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

SegmenterInstanceInterface* TextwolfSegmenter::createInstance( const analyzer::SegmenterOptions& opts) const
{
	try
	{
		if (!opts.items().empty()) throw strus::runtime_error(_TXT("no options defined for segmenter '%s'"), SEGMENTER_NAME);
		return new TextwolfSegmenterInstance( m_errorhnd);
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' segmenter: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

ContentIteratorInterface* TextwolfSegmenter::createContentIterator(
		const char* content,
		std::size_t contentsize,
		const std::vector<std::string>& attributes,
		const std::vector<std::string>& expressions,
		const analyzer::DocumentClass& dclass,
		const analyzer::SegmenterOptions &opts) const
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

		if (!opts.items().empty())
		{
			throw strus::runtime_error(_TXT("no options defined for segmenter '%s'"), SEGMENTER_NAME);
		}
		if (dclass.encoding().empty())
		{
			return new ContentIterator<UTF8>( content, contentsize, attributes, expressions, UTF8(), m_errorhnd);
		}
		else
		{
			if (strus::caseInsensitiveStartsWith( dclass.encoding(), "IsoLatin")
			||  strus::caseInsensitiveStartsWith( dclass.encoding(), "ISO-8859"))
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
				return new ContentIterator<IsoLatin>( content, contentsize, attributes, expressions, IsoLatin(codepage), m_errorhnd);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UTF-8"))
			{
				return new ContentIterator<UTF8>( content, contentsize, attributes, expressions, UTF8(), m_errorhnd);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UTF-16")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-16BE"))
			{
				return new ContentIterator<UTF16BE>( content, contentsize, attributes, expressions, UTF16BE(), m_errorhnd);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UTF-16LE"))
			{
				return new ContentIterator<UTF16LE>( content, contentsize, attributes, expressions, UTF16LE(), m_errorhnd);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-2")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UCS-2BE"))
			{
				return new ContentIterator<UCS2BE>( content, contentsize, attributes, expressions, UCS2BE(), m_errorhnd);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-2LE"))
			{
				return new ContentIterator<UCS2LE>( content, contentsize, attributes, expressions, UCS2LE(), m_errorhnd);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-4")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UCS-4BE")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-32")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-32BE"))
			{
				return new ContentIterator<UCS4BE>( content, contentsize, attributes, expressions, UCS4BE(), m_errorhnd);
			}
			else if (strus::caseInsensitiveEquals( dclass.encoding(), "UCS-4LE")
			||       strus::caseInsensitiveEquals( dclass.encoding(), "UTF-32LE"))
			{
				return new ContentIterator<UCS4LE>( content, contentsize, attributes, expressions, UCS4LE(), m_errorhnd);
			}
			else
			{
				throw strus::runtime_error( _TXT("only support for UTF-8,UTF-16BE,UTF-16LE,UTF-32BE,UCS-4BE,UTF-32LE,UCS-4LE and ISO-8859 (code pages 1 to 9) as character set encoding"));
			}
		}
	}
	CATCH_ERROR_ARG1_MAP_RETURN( _TXT("error in '%s' segmenter creating content iterator: %s"), SEGMENTER_NAME, *m_errorhnd, 0);
}

StructView TextwolfSegmenter::view() const
{
	try
	{
		return StructView()
			("name", name())
			("mimetype", mimeType())
			("description", _TXT("Segmenter for XML (application/xml) based on the textwolf library"))
		;
	}
	CATCH_ERROR_MAP_RETURN( _TXT("error in introspection: %s"), *m_errorhnd, StructView());
}


