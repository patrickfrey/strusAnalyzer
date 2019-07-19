/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Structure describing the internal representation of a normalizer/tokenizer/aggregator function in the analyzer
/// \note The internal representation may not be suitable for reconstructing the object
/// \file functionView.hpp
#ifndef _STRUS_ANALYZER_FUNCTION_VIEW_HPP_INCLUDED
#define _STRUS_ANALYZER_FUNCTION_VIEW_HPP_INCLUDED
#include "strus/base/enable_if.hpp"
#include "strus/base/type_traits.hpp"
#include "strus/numericVariant.hpp"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

/// \brief strus toplevel namespace
namespace strus {
/// \brief analyzer parameter and return value objects namespace
namespace analyzer {

/// \brief Structure describing the internal representation of a normalizer/tokenizer/aggregator function in the analyzer
/// \note The internal representation may not be suitable for reconstructing the object
class FunctionView
{
public:
	typedef std::pair<std::string,std::string> NamedParameter;

	/// \brief Default constructor
	FunctionView(){}
	/// \brief Copy constructor
	FunctionView( const FunctionView& o)
		:m_name(o.m_name),m_parameter(o.m_parameter){}
	/// \brief Constructor
	/// \param[in] name_ name of the function
	/// \param[in] params_ list of named parameters
	FunctionView( const std::string& name_, const std::vector<NamedParameter>& params_)
		:m_name(name_),m_parameter(params_){}
	/// \brief Constructor
	/// \param[in] name_ name of the function
	/// \param[in] params_ list of named parameters
	explicit FunctionView( const std::string& name_)
		:m_name(name_),m_parameter(){}

	/// \brief Conditional for atomic type
	template<typename type>
	struct is_atomic
	{
		typename strus::enable_if<
			strus::is_arithmetic<type>::value
			|| strus::is_same<bool,type>::value
			|| strus::is_same<std::string,type>::value
			|| strus::is_same<char*,type>::value
		,bool> value_type;
		enum {value=sizeof(value_type)};
	};

	/// \brief Operator to build parameter list (for atomic value type)
	/// \param[in] name_ name of the parameter
	/// \param[in] value_ value of the parameter
	template <typename value_type>
	typename strus::enable_if<is_atomic<value_type>::value,FunctionView&>::type operator()( const char* name_, const value_type& value_)
	{
		std::ostringstream out;
		out << value_;
		m_parameter.push_back( NamedParameter( name_, out.str()));
		return *this;
	}

	/// \brief Operator to build parameter list (for map of string to atomic value type)
	/// \param[in] name_ name of the parameter
	/// \param[in] value_ value of the parameter
	template <typename value_type>
	typename strus::enable_if<is_atomic<value_type>::value,FunctionView&>::type operator()( const char* name_, const std::map<std::string,value_type>& value_)
	{
		typename std::map<std::string,value_type>::const_iterator vi = value_.begin(), ve = value_.end();
		for (; vi != ve; ++vi)
		{
			std::ostringstream out;
			out << vi->second;
			m_parameter.push_back( NamedParameter( std::string(name_)+"::"+vi->first, out.str()));
		}
		return *this;
	}

	/// \brief Operator to build parameter list (for map of string to atomic value type)
	/// \param[in] name_ name of the parameter
	/// \param[in] value_ value of the parameter
	template <typename value_type>
	typename strus::enable_if<is_atomic<value_type>::value,FunctionView&>::type operator()( const char* name_, const std::vector<value_type>& value_)
	{
		typename std::vector<value_type>::const_iterator vi = value_.begin(), ve = value_.end();
		for (; vi != ve; ++vi)
		{
			std::ostringstream out;
			out << *vi;
			m_parameter.push_back( NamedParameter( name_, out.str()));
		}
		return *this;
	}

	/// \brief Operator to build parameter list (for value type numeric variant)
	/// \param[in] name_ name of the parameter
	/// \param[in] value_ value of the parameter
	template <typename value_type>
	typename strus::enable_if<strus::is_same<NumericVariant,value_type>::value,FunctionView&>::type operator()( const char* name_, const value_type& value_)
	{
		std::ostringstream out;
		out << value_.tostring().c_str();
		m_parameter.push_back( NamedParameter( name_, out.str()));
		return *this;
	}

	/// \brief Get the name of the function
	/// \return name of the function
	const std::string& name() const				{return m_name;}

	/// \brief Get the internal representation of the named parameters of the function
	/// \note The parameter list is not the list of parameters passed on the construction of the object but a view to the internal representation of the parameters of the function
	/// \return elements of the function
	const std::vector<NamedParameter>& parameter() const	{return m_parameter;}

private:
	std::string m_name;
	std::vector<NamedParameter> m_parameter;
};

}}//namespace
#endif

