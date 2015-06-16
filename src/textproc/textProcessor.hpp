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
#ifndef _STRUS_ANALYZER_TEXT_PROCESSOR_HPP_INCLUDED
#define _STRUS_ANALYZER_TEXT_PROCESSOR_HPP_INCLUDED
#include "strus/textProcessorInterface.hpp"
#include <map>
#include <vector>

namespace strus {

class TextProcessor
	:public TextProcessorInterface
{
public:
	TextProcessor();

	virtual void addResourcePath( const std::string& path);
	virtual std::string getResourcePath( const std::string& filename) const;

	virtual const TokenizerFunctionInterface* getTokenizer( const std::string& name) const;

	virtual const NormalizerFunctionInterface* getNormalizer( const std::string& name) const;

	virtual const StatisticsFunctionInterface* getStatisticsFunction( const std::string& name) const;

	virtual void defineTokenizer( const std::string& name, const TokenizerFunctionInterface* tokenizer);

	virtual void defineNormalizer( const std::string& name, const NormalizerFunctionInterface* normalizer);

	virtual void defineStatisticsFunction( const std::string& name, const StatisticsFunctionInterface* statfunc);

private:
	std::map<std::string,const TokenizerFunctionInterface*> m_tokenizer_map;
	std::map<std::string,const NormalizerFunctionInterface*> m_normalizer_map;
	std::map<std::string,const StatisticsFunctionInterface*> m_statistics_map;
	std::vector<std::string> m_resourcePaths;
};

}//namespace
#endif

