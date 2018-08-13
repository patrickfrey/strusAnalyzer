/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_CONTENT_ITERATOR_CJSON_HPP_INCLUDED
#define _STRUS_SEGMENTER_CONTENT_ITERATOR_CJSON_HPP_INCLUDED
#include "strus/contentIteratorInterface.hpp"
#include "strus/analyzer/segmenterOptions.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/reference.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"
#include "private/contentIteratorStm.hpp"
#include "private/textEncoder.hpp"
#include "textwolf/xmlscanner.hpp"
#include "cjson/cJSON.h"
#include "cjson2textwolf.hpp"
#include <cstdlib>
#include <vector>
#include <set>
#include <setjmp.h>

#define SEGMENTER_NAME "cjson"

namespace strus
{

class ContentIterator
	:public ContentIteratorInterface
{
public:
	ContentIterator( 
			const char* content_,
			std::size_t contentsize_,
			const std::vector<std::string>& attributes_,
			const strus::Reference<strus::utils::TextEncoderBase>& encoder_,
			ErrorBufferInterface* errorhnd_);

	virtual ~ContentIterator()
	{
		clear();
	}

	virtual bool getNext(
			const char*& expression, std::size_t& expressionsize,
			const char*& segment, std::size_t& segmentsize);

private:
	void clear();

private:
	ErrorBufferInterface* m_errorhnd;
	std::set<std::string> m_attributes;
	std::string m_content;
	strus::Reference<strus::utils::TextEncoderBase> m_encoder;
	std::vector<TextwolfItem> m_ar;
	std::vector<TextwolfItem>::const_iterator m_elemitr;
	cJSON* m_tree;
	ContentIteratorStm m_stm;
	std::string m_path;
	bool m_eof;
};

}//namespace
#endif


