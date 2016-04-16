/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Some utility functions that are centralised to control dependencies to boost
#ifndef _STRUS_UTILS_HPP_INCLUDED
#define _STRUS_UTILS_HPP_INCLUDED
#include "strus/numericVariant.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace strus {
namespace utils {

std::string tolower( const std::string& val);
std::string trim( const std::string& val);
bool caseInsensitiveEquals( const std::string& val1, const std::string& val2);
bool caseInsensitiveStartsWith( const std::string& val, const std::string& prefix);
int toint( const std::string& val);
std::string tostring( int val);
bool isFile( const std::string& path);

template <class X>
class SharedPtr
	:public boost::shared_ptr<X>
{
public:
	SharedPtr( X* ptr)
		:boost::shared_ptr<X>(ptr){}
	SharedPtr( const SharedPtr& o)
		:boost::shared_ptr<X>(o){}
	SharedPtr()
		:boost::shared_ptr<X>(){}
};

template <class X>
class ScopedPtr
	:public boost::scoped_ptr<X>
{
public:
	ScopedPtr( X* ptr)
		:boost::scoped_ptr<X>(ptr){}
	ScopedPtr( const ScopedPtr& o)
		:boost::scoped_ptr<X>(o){}
	ScopedPtr()
		:boost::scoped_ptr<X>(){}
};

}} //namespace
#endif


