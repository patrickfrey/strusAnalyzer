/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_JSON_PARSER_HPP_INCLUDED
#define _STRUS_ANALYZER_JSON_PARSER_HPP_INCLUDED
#include "strus/errorBufferInterface.hpp"

namespace strus
{

/// \brief Return the pointer to the end of the next document in a JSON document list
/// \param[in] str pointer to UTF-8 source start
/// \return the start of the next document or NULL in case of error
const char* jsonSkipEndOfNextDocument( const char* str);

}
#endif
