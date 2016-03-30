/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_NORMALIZER_DICTIONARY_MAP_HPP_INCLUDED
#define _STRUS_NORMALIZER_DICTIONARY_MAP_HPP_INCLUDED
#include "compactNodeTrie.hpp"
#include "strus/normalizerFunctionContextInterface.hpp"
#include "strus/normalizerFunctionInterface.hpp"
#include "strus/normalizerFunctionInstanceInterface.hpp"
#include "strus/textProcessorInterface.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace strus
{
/// \brief Forward declaration
class AnalyzerErrorBufferInterface;

class DictMapNormalizerFunction
	:public NormalizerFunctionInterface
{
public:
	explicit DictMapNormalizerFunction( AnalyzerErrorBufferInterface* errorhnd_)
		:m_errorhnd(errorhnd_){}

	virtual NormalizerFunctionInstanceInterface* createInstance( const std::vector<std::string>& args, const TextProcessorInterface*) const;

	virtual const char* getDescription() const
	{
		return "Normalizer mapping the elements with a dictionary. For found elements the passed value is returned. The dictionary file name is passed as argument";
	}
	
private:
	AnalyzerErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif

