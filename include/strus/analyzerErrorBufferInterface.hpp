/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Interface for reporting and catching errors
/// \file analyzerErrorBufferInterface.hpp
#ifndef _STRUS_ANALYZER_ERROR_BUFFER_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_ERROR_BUFFER_INTERFACE_HPP_INCLUDED

/// \brief strus toplevel namespace
namespace strus
{

/// \class AnalyzerErrorBufferInterface
/// \brief Interface for reporting and catching errors in the analyzer
class AnalyzerErrorBufferInterface
{
public:
	enum ErrorClass
	{
		None,		///< no error
		RuntimeError,	///< runtime error
		BadAlloc	///< memory allocation error
	};

	/// \brief Destructor
	virtual ~AnalyzerErrorBufferInterface(){}

	/// \brief Report an error
	/// \param[in] format error message format string
	/// \remark must not throw
	virtual void report( const char* format, ...) const=0;

	/// \brief Report an error, overwriting the previous error
	/// \param[in] format error message format string
	/// \remark must not throw
	virtual void explain( const char* format) const=0;

	/// \brief Check, if an error has occurred and return it
	/// \return an error string, if defined, NULL else
	/// \remark resets the error
	virtual const char* fetchError()=0;

	/// \brief Check, if an error has occurred
	/// \return an error string, if defined, NULL else
	virtual bool hasError() const=0;
};

}//namespace
#endif

