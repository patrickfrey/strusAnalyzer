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
#include "strus/analyzerInterface.hpp"
#include "strus/analyzerLib.hpp"
#include "strus/tokenMiner.hpp"
#include "strus/tokenMinerFactory.hpp"
#include "strus/tokenMinerLib.hpp"
#include "strus/storageLib.hpp"
#include "strus/storageInterface.hpp"
#include "system/fileio.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <boost/scoped_ptr.hpp>

static int failedOperations = 0;
static int succeededOperations = 0;

static bool processQuery( 
	strus::StorageInterface* storage,
	const strus::AnalyzerInterface* analyzer,
	const std::string& querystring)
{
	try
	{
		std::vector<strus::AnalyzerInterface::Term> termar
			= analyzer->analyze( querystring);

		std::vector<strus::AnalyzerInterface::Term>::const_iterator
			ti = termar.begin(), te = termar.end();

		boost::scoped_ptr<strus::StorageInterface::TransactionInterface>
			transaction( storage->createTransaction( path));

		transaction->setDocumentAttribute( 'D', path);

		for (; ti != te; ++ti)
		{
			transaction->addTermOccurrence(
				ti->type(), ti->value(), ti->pos());
		}
		transaction->commit();
		std::cerr << "inserted document '" << path << "'" << std::endl;
		return true;
	}
	catch (const std::runtime_error& err)
	{
		std::cerr << "ERROR failed to insert document '" << path << "': " << err.what() << std::endl;
		return false;
	}
}

int main( int argc, const char* argv[])
{

	if (argc > 4)
	{
		std::cerr << "ERROR too many arguments" << std::endl;
	}
	if (argc < 4)
	{
		std::cerr << "ERROR too few arguments" << std::endl;
	}
	if (argc != 4 || std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0)
	{
		std::cerr << "usage: strusQuery <program> <config> <qrypath>" << std::endl;
		std::cerr << "<program>     = path of query analyzer program" << std::endl;
		std::cerr << "<config>      = storage configuration string as used for strusCreate" << std::endl;
		std::cerr << "<qrypath>     = path of query or '-' for stdin";
		return 0;
	}
	try
	{
		unsigned int ec;
		std::string analyzerProgramSource;
		ec = strus::readFile( argv[1], analyzerProgramSource);
		if (ec)
		{
			std::ostringstream msg;
			std::cerr << "ERROR failed to load analyzer program " << argv[1] << " (file system error " << ec << ")" << std::endl;
			return 2;
		}
		std::string tokenMinerSource;
		boost::scoped_ptr<strus::TokenMinerFactory> minerfac(
			strus::createTokenMinerFactory( tokenMinerSource));

		boost::scoped_ptr<strus::AnalyzerInterface> analyzer(
			strus::createAnalyzer( *minerfac, analyzerProgramSource));

		boost::scoped_ptr<strus::StorageInterface> storage(
					strus::createStorageClient( argv[2]));

		std::string querystring;
		if (path == "-")
		{
			ec = strus::readStdin( querystring);
			if (ec)
			{
				std::cerr << "ERROR failed to read query string from stdin" << std::endl;
				return 3;
			}
		}
		else
		{
			ec = strus::readFile( path, querystring);
			if (ec)
			{
				std::cerr << "ERROR failed to read query string from file '" << path << "'" << std::endl;
				return 4;
			}
		}
		if (!processQuery( storage.get(), analyzer.get(), querystring))
		{
			std::cerr << "ERROR query evaluation failed" << std::endl;
			return 5;
		}
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "ERROR " << e.what() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "EXCEPTION " << e.what() << std::endl;
	}
	return -1;
}


