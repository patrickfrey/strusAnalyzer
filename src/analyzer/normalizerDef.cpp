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
#include "normalizerDef.hpp"
#include "strus/normalizerConfig.hpp"
#include "strus/textProcessorInterface.hpp"
#include <stdexcept>

using namespace strus;

std::vector<NormalizerDef> NormalizerDef::getNormalizerDefList( const TextProcessorInterface* tp, const std::vector<NormalizerConfig>& config)
{
	std::vector<NormalizerDef> rt;
	std::vector<NormalizerConfig>::const_iterator
		ci = config.begin(), ce = config.end();
	for (; ci != ce; ++ci)
	{
		const NormalizerInterface* nm = tp->getNormalizer( ci->name());
		utils::SharedPtr<NormalizerInterface::Argument>
			nmarg( nm->createArgument( tp, ci->arguments()));
	
		if (!nmarg.get() && !ci->arguments().empty())
		{
			throw std::runtime_error( std::string( "no arguments expected for normalizer '") + ci->name() + "'");
		}
		rt.push_back( NormalizerDef( nm, nmarg));
	}
	return rt;
}


