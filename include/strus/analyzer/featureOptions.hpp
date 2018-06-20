/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Options to stear the position assignment of features
/// \file featureOptions.hpp
#ifndef _STRUS_ANALYZER_FEATURE_OPTIONS_HPP_INCLUDED
#define _STRUS_ANALYZER_FEATURE_OPTIONS_HPP_INCLUDED
#include "strus/analyzer/positionBind.hpp"
#include <utility>

/// \brief strus toplevel namespace
namespace strus {
namespace analyzer {

/// \class FeatureOptions
/// \brief Options to stear the creation of terms in the analyzer
class FeatureOptions
{
public:
	/// \brief Default constructor
	FeatureOptions()
		:m_opt(0){}
	/// \brief Copy constructor
#if __cplusplus >= 201103L
	FeatureOptions( FeatureOptions&& ) = default;
	FeatureOptions( const FeatureOptions& ) = default;
	FeatureOptions& operator= ( FeatureOptions&& ) = default;
	FeatureOptions& operator= ( const FeatureOptions& ) = default;
#else
	FeatureOptions( const FeatureOptions& o)
		:m_opt(o.m_opt){}
#endif
	/// \brief Constructor
	FeatureOptions( unsigned int opt_)
		:m_opt(opt_){}

	/// \brief Get the PositionBind value set
	analyzer::PositionBind positionBind() const		{return (analyzer::PositionBind)(m_opt & 0x3);}

	/// \brief Define the PositionBind value
	void definePositionBind( analyzer::PositionBind b)	{m_opt &= ~0x3; m_opt |= (unsigned int)b;}

	/// \brief Get the options transacription as integer
	unsigned int opt() const				{return m_opt;}

private:
	unsigned int m_opt;
};

}}//namespace
#endif

