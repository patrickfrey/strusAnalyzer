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
#include "strus/segmenterInterface.hpp"
#include "strus/lib/segmenter_textwolf.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <map>
#include <boost/scoped_ptr.hpp>

#undef STRUS_LOWLEVEL_DEBUG

int main( int , const char** )
{
	try
	{
		boost::scoped_ptr<strus::SegmenterInterface> segmenter( strus::createSegmenter_textwolf());

		segmenter->defineSelectorExpression( 1, "/doc/person/name()");
		segmenter->defineSelectorExpression( 2, "/doc/person/birth()");
		segmenter->defineSelectorExpression( 3, "/doc/person()");
		segmenter->defineSelectorExpression( 4, "/doc/person@id");
		segmenter->defineSelectorExpression( 11, "/doc/company/name()");
		segmenter->defineSelectorExpression( 12, "/doc/company/birth()");
		segmenter->defineSelectorExpression( 13, "/doc/company()");
		segmenter->defineSelectorExpression( 14, "/doc/company");
		segmenter->defineSelectorExpression( 15, "/doc/company@id");
		segmenter->defineSelectorExpression( 16, "/doc/company@num");
		segmenter->defineSelectorExpression( 21, "/doc/place/name()");
		segmenter->defineSelectorExpression( 22, "/doc/place/birth()");
		segmenter->defineSelectorExpression( 23, "/doc/place()");
		segmenter->defineSelectorExpression( 25, "/doc/place@id");
		segmenter->defineSelectorExpression( 24, "/doc/place");
		segmenter->defineSelectorExpression( 31, "/doc/company/name()");
		segmenter->defineSelectorExpression( 32, "/doc/company/birth()");
		segmenter->defineSelectorExpression( 33, "/doc/company()");
		segmenter->defineSelectorExpression( 35, "/doc/company@id");
		segmenter->defineSelectorExpression( 36, "/doc/company@num");
		segmenter->defineSelectorExpression( 34, "/doc/company");

		std::size_t idar[] = {1,2,3,4,11,12,13,14,15,16,21,22,23,24,25,31,32,33,34,35,36,0};
		typedef std::map<std::size_t,strus::SegmenterInterface::SelectorType> ExpectedMap;
		ExpectedMap expected;
		expected[  1] = strus::SegmenterInterface::Content;
		expected[  2] = strus::SegmenterInterface::Content;
		expected[  3] = strus::SegmenterInterface::Content;
		expected[  4] = strus::SegmenterInterface::AnnotationSuccessor;
		expected[ 11] = strus::SegmenterInterface::Content;
		expected[ 12] = strus::SegmenterInterface::Content;
		expected[ 13] = strus::SegmenterInterface::Content;
		expected[ 14] = strus::SegmenterInterface::Content;
		expected[ 15] = strus::SegmenterInterface::AnnotationPredecessor;
		expected[ 16] = strus::SegmenterInterface::AnnotationPredecessor;
		expected[ 21] = strus::SegmenterInterface::Content;
		expected[ 22] = strus::SegmenterInterface::Content;
		expected[ 23] = strus::SegmenterInterface::Content;
		expected[ 24] = strus::SegmenterInterface::Content;
		expected[ 25] = strus::SegmenterInterface::AnnotationPredecessor;
		expected[ 31] = strus::SegmenterInterface::Content;
		expected[ 32] = strus::SegmenterInterface::Content;
		expected[ 33] = strus::SegmenterInterface::Content;
		expected[ 34] = strus::SegmenterInterface::Content;
		expected[ 35] = strus::SegmenterInterface::AnnotationPredecessor;
		expected[ 36] = strus::SegmenterInterface::AnnotationPredecessor;

		std::size_t ii = 0;
		for (; idar[ii]; ++ii)
		{
			strus::SegmenterInterface::SelectorType
				res = segmenter->getSelectorType( idar[ ii]),
				exp = expected[ idar[ ii]];
			std::cout 
				<< idar[ ii] << ": "
				<< strus::SegmenterInterface::selectorTypeName( res)
				<< std::endl;

			if (exp != res)
			{
				throw std::runtime_error( std::string("test failed, expected '")
						+ strus::SegmenterInterface::selectorTypeName( exp)
						+ "'");
			}
		}
		std::cerr << "Ok. Annotation position test passed." << std::endl;
		return 0;
	}
	catch (const std::exception& err)
	{
		std::cerr << "EXCEPTION " << err.what() << std::endl;
	}
	return -1;
}


