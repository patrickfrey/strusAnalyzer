/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of context to markup documents with tags derived from POS tagging
/// \file posTaggerContext.cpp
#include "posTaggerContext.hpp"
#include "strus/errorBufferInterface.hpp"
#include "strus/debugTraceInterface.hpp"
#include "private/internationalization.hpp"
#include "private/errorUtils.hpp"

using namespace strus;

PosTagger::PosTagger( ErrorBufferInterface* errorhnd_)
	:m_errorhnd(errorhnd_)
{}

PosTagger::~PosTagger()
{}


