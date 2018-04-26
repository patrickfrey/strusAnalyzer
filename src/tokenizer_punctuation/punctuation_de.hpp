/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_TOKENIZER_PUNCTUATION_DE_HPP_INCLUDED
#define _STRUS_TOKENIZER_PUNCTUATION_DE_HPP_INCLUDED
#include "strus/tokenizerFunctionInstanceInterface.hpp"
#include "strus/introspectionInterface.hpp"
#include "punctuation_utils.hpp"

namespace strus
{
/// \brief Forward declaration
class ErrorBufferInterface;

class PunctuationTokenizerInstance_de
	:public TokenizerFunctionInstanceInterface
{
public:
	PunctuationTokenizerInstance_de(
			const char* punctuationCharList,
			ErrorBufferInterface* errorhnd)
		:m_punctuation_char(punctuationCharList?punctuationCharList:":.;,!?()-")
		,m_punctuation_charlist(punctuationCharList?(punctuationCharList[0]?punctuationCharList:"."):":.;,!?()-")
		,m_errorhnd(errorhnd){}

	virtual bool concatBeforeTokenize() const
	{
		return true;
	}

	inline bool isPunctuation( textwolf::UChar ch) const
	{
		return (ch <= 127 && (m_punctuation_char)[(unsigned char)ch]);
	}

	virtual std::vector<analyzer::Token> tokenize( const char* src, std::size_t srcsize) const;

	virtual IntrospectionInterface* createIntrospection() const;

private:
	CharTable m_punctuation_char;
	std::string m_punctuation_charlist;
	ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

