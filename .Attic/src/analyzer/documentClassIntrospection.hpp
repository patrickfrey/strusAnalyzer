/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of the introspection for the analyzer document class
/// \file documentClassIntrospection.hpp
#ifndef _STRUS_ANALYZER_DOCUMENT_CLASS_INTROSPECTION_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCUMENT_CLASS_INTROSPECTION_HPP_INCLUDED
#include "strus/introspectionInterface.hpp"
#include "strus/analyzer/documentClass.hpp"
#include "strus/errorBufferInterface.hpp"

namespace strus
{

class DocumentClassIntrospection
	:public IntrospectionInterface
{
public:
	DocumentClassIntrospection( const analyzer::DocumentClass* documentClass_, ErrorBufferInterface* errorhnd_)
		:m_documentClass(documentClass_),m_errorhnd(errorhnd_){}
	virtual ~DocumentClassIntrospection( ){}

	virtual IntrospectionInterface* open( const std::string& name) const;
	virtual std::string value() const;

	virtual std::vector<std::string> list() const;
	static IntrospectionInterface* constructor( const void* self_, class ErrorBufferInterface* errorhnd_);

private:
	const analyzer::DocumentClass* m_documentClass;
	mutable ErrorBufferInterface* m_errorhnd;
};

}//namespace
#endif


