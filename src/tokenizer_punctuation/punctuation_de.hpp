/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

