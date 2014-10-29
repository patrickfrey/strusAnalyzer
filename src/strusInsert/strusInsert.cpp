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

static bool processDocument( 
	strus::StorageInterface* storage,
	const strus::AnalyzerInterface* analyzer,
	const std::string& path)
{
	try
	{
		std::string documentContent;
		unsigned int ec = strus::readFile( path, documentContent);
		if (ec)
		{
			std::ostringstream msg;
			std::cerr << "failed to load document to analyze " << path << " (file system error '" << ec << ")" << std::endl;
			return false;
		}

		std::vector<strus::AnalyzerInterface::Term> termar
			= analyzer->analyze( documentContent);

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
		return true;
	}
	catch (const std::runtime_error& err)
	{
		std::cerr << "Failed to insert document '" << path << "': " << err.what() << std::endl;
		return false;
	}
}

int main( int argc, const char* argv[])
{
	int failedOperations = 0;
	int succeededOperations = 0;

	if (argc > 3)
	{
		std::cerr << "ERROR too many arguments" << std::endl;
	}
	if (argc < 3)
	{
		std::cerr << "ERROR too few arguments" << std::endl;
	}
	if (argc != 3 || std::strcmp( argv[1], "-h") == 0 || std::strcmp( argv[1], "--help") == 0)
	{
		std::cerr << "usage: strusInsert <program> <config> <docpath>" << std::endl;
		std::cerr << "<program>     = path of analyzer program" << std::endl;
		std::cerr << "<config>      = storage configuration string" << std::endl;
		std::cerr << "<docpath>     = path of document or directory to insert: ";
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
			std::cerr << "ERROR failed to load analyzer program " << argv[1] << " (file system error '" << ec << ")" << std::endl;
			return 2;
		}
		std::string tokenMinerSource;
		boost::scoped_ptr<strus::TokenMinerFactory> minerfac(
			strus::createTokenMinerFactory( tokenMinerSource));

		boost::scoped_ptr<strus::AnalyzerInterface> analyzer(
			strus::createAnalyzer( *minerfac, analyzerProgramSource));

		boost::scoped_ptr<strus::StorageInterface> storage(
					strus::createStorageClient( argv[2]));

		std::string path( argv[3]);
		std::vector<std::string> filesToProcess;
		if (strus::isDir( path))
		{
			unsigned int ec = strus::readDir( path, ".xml", filesToProcess);
			if (ec)
			{
				std::cerr << "ERROR could not read directory to process '" << path << "' (file system error '" << ec << ")" << std::endl;
				return 3;
			}
			std::vector<std::string>::const_iterator pi = filesToProcess.begin(), pe = filesToProcess.end();
			for (; pi != pe; ++pi)
			{
				std::string file( path + strus::dirSeparator() + *pi);
				if (processDocument( storage.get(), analyzer.get(), path))
				{
					++succeededOperations;
				}
				else
				{
					++failedOperations;
				}
			}
		}
		else if (strus::isFile( path))
		{
			if (processDocument( storage.get(), analyzer.get(), path))
			{
				++succeededOperations;
			}
			else
			{
				++failedOperations;
			}
		}
		if (failedOperations > 0)
		{
			std::cerr << "total " << failedOperations << " inserts failed out of " << (succeededOperations + failedOperations) << std::endl;
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


