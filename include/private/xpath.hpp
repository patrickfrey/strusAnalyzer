/*
 * Copyright (c) 2019 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_XPATH_HELPER_FUNCTIONS_HPP_INCLUDED
#define _STRUS_XPATH_HELPER_FUNCTIONS_HPP_INCLUDED
#include <string>

namespace strus
{

/// \brief Create an xpath expression by concating two expressions logically
/// \param[in] parent parent path
/// \param[in] follow path relative from parent
/// \note The 2nd argument path is considered to be relative to parent, even if it starts with '/'
std::string joinXPathExpression( const std::string& parent, const std::string& follow);

/// \brief Cut '()' at end of expression
std::string xpathStartStructurePath( const std::string& selectexpr);

/// \brief Append unique '~' at end of expression
std::string xpathEndStructurePath( const std::string& selectexpr);

}//namespace
#endif



