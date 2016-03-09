/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2015 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#ifndef _STRUS_TOKENIZER_PUNCTUATION_DE_HPP_INCLUDED
#define _STRUS_TOKENIZER_PUNCTUATION_DE_HPP_INCLUDED
#include "strus/tokenizerFunctionContextInterface.hpp"
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "punctuation_utils.hpp"

namespace strus
{
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

class PunctuationTokenizerFunctionContext_de
	:public TokenizerFunctionContextInterface
{
public:
	PunctuationTokenizerFunctionContext_de(
			const CharTable* punctuation_char_,
			AnalyzerErrorBufferInterface* errorhnd)
		:m_punctuation_char(punctuation_char_)
		,m_errorhnd(errorhnd){}

	inline bool isPunctuation( textwolf::UChar ch)
	{
		return (ch <= 127 && (*m_punctuation_char)[(unsigned char)ch]);
	}

	virtual std::vector<analyzer::Token> tokenize( const char* src, std::size_t srcsize);

private:
	const CharTable* m_punctuation_char;
	AnalyzerErrorBufferInterface* m_errorhnd;
};


class PunctuationTokenizerInstance_de
	:public TokenizerFunctionInstanceInterface
{
public:
	PunctuationTokenizerInstance_de(
			const char* punctuationCharList,
			AnalyzerErrorBufferInterface* errorhnd)
		:m_punctuation_char(punctuationCharList?punctuationCharList:":.;,!?()-")
		,m_errorhnd(errorhnd){}

	virtual bool concatBeforeTokenize() const
	{
		return true;
	}

	TokenizerFunctionContextInterface* createFunctionContext() const;

private:
	CharTable m_punctuation_char;
	AnalyzerErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

