/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_MAP_CJSON_TO_TEXTWOLF_HPP_INCLUDED
#define _STRUS_SEGMENTER_MAP_CJSON_TO_TEXTWOLF_HPP_INCLUDED
#include "textwolf/xmlscanner.hpp"
#include "cjson/cJSON.h"
#include <cstdlib>
#include <vector>
#include <string>

#define SEGMENTER_NAME "cjson"

namespace strus
{

struct TextwolfItem
{
	typedef textwolf::XMLScannerBase::ElementType Type;
	Type type;
	const char* value;

	TextwolfItem( const TextwolfItem& o)
		:type(o.type),value(o.value){}
	explicit TextwolfItem( const Type& type_, const char* value_=0)
		:type(type_),value(value_){}
};


void getTextwolfItems( std::vector<TextwolfItem>& itemar, cJSON const* nd);

cJSON* parseJsonTree( const std::string& content);

}//namespace
#endif


