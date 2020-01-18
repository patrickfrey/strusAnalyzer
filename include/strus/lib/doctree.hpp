/*
 * Copyright (c) 2020 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Functions to read and process a document as tree structure
/// \note The XML implementation is based on the textwolf template library
/// \note The JSON implementation is based on the cjson library
/// \file doctree.hpp
#ifndef _STRUS_ANALYZER_DOCTREE_LIB_HPP_INCLUDED
#define _STRUS_ANALYZER_DOCTREE_LIB_HPP_INCLUDED
#include "strus/reference.hpp"
#include <string>
#include <list>
#include <utility>

/// \brief strus toplevel namespace
namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;

/// \brief Representation of a tree structure capable of representing an XML/JSON document
class DocTree
{
public:
	/// \brief Tag attribute
	class Attribute
	{
	public:
		/// \brief Default constructor
		Attribute()
			:m_name(),m_value(){}
		/// \brief Constructor
		Attribute( const std::string& name_, const std::string& value_)
			:m_name(name_),m_value(value_){}
		/// \brief Copy constructor
		Attribute( const Attribute& o)
			:m_name(o.m_name),m_value(o.m_value){}
#if __cplusplus >= 201103L
		Attribute( std::string&& name_, std::string&& value_)
			:m_name(std::move(name_)),m_value(std::move(value_)){}
		Attribute( Attribute&& o)
			:m_name(std::move(o.m_name)),m_value(std::move(o.m_value)){}
		Attribute& operator= ( Attribute&& o)
			{m_name=std::move(o.m_name);m_value=std::move(o.m_value); return *this;}
#endif
		~Attribute(){}

		/// \brief Get the attribute name
		const std::string& name() const
			{return m_name;}
		/// \brief Get the attribute value
		const std::string& value() const
			{return m_value;}

	private:
		std::string m_name;
		std::string m_value;
	};

public:
	/// \brief Default constructor
	DocTree()
		:m_name(),m_value(),m_attr(),m_chld(){}
	/// \brief Copy constructor
	DocTree( const DocTree& o)
		:m_name(o.m_name),m_value(o.m_value),m_attr(o.m_attr),m_chld(o.m_chld){}
	/// \brief Constructor
	explicit DocTree( const std::string& name_, const std::string& value_=std::string(), const std::list<Attribute>& attr_=std::list<Attribute>(), const std::list<strus::Reference<DocTree> >& chld_=std::list<strus::Reference<DocTree> >())
		:m_name(name_),m_value(value_),m_attr(attr_),m_chld(chld_){}
#if __cplusplus >= 201103L
	DocTree( std::string&& name_, std::string&& value_, std::list<Attribute>&& attr_, std::list<strus::Reference<DocTree> >&& chld_)
		:m_name(std::move(name_)),m_value(std::move(value_)),m_attr(std::move(attr_)),m_chld(std::move(chld_)){}
	DocTree( DocTree&& o)
		:m_name(std::move(o.m_name)),m_value(std::move(o.m_value)),m_attr(std::move(o.m_attr)),m_chld(std::move(o.m_chld)){}
	DocTree& operator= ( DocTree&& o)
		{m_name=std::move(o.m_name);m_value=std::move(o.m_value);m_attr=std::move(o.m_attr);m_chld=std::move(o.m_chld); return *this;}
#endif
	~DocTree(){}


public:
	/// \brief Get the top level tree element name
	const std::string& name() const
		{return m_name;}
	/// \brief Set the top level tree element name
	void setName( const std::string& name_)
		{m_name = name_;}

	/// \brief Get the top level tree element content value
	const std::string& value() const
		{return m_value;}
	/// \brief Set the top level tree element content value
	void setValue( const std::string& value_)
		{m_value = value_;}

	/// \brief Get the top level tree element tag attributes
	const std::list<Attribute>& attr() const
		{return m_attr;}
	/// \brief Add a top level tree element tag attribute
	void addAttr( const Attribute& attribute_)
		{m_attr.push_back( attribute_);}
	/// \brief Add a top level tree element tag attribute
	void addAttr( const std::string& name_, const std::string& value_)
		{m_attr.push_back( Attribute( name_, value_));}
	/// \brief Get the top level tree element sub nodes
	const std::list<strus::Reference<DocTree> >& chld() const
		{return m_chld;}
	/// \brief Add a top level tree element sub node
	void addChld( const DocTree& nd)
		{m_chld.push_back( new DocTree( nd));}
	/// \brief Add a top level tree element sub node
	/// \param[in] nd ponter to tree (ownership passed)
	void addChld( DocTree* nd)
		{m_chld.push_back( nd);}
	/// \brief Add a top level tree element sub node
	void addChld( const strus::Reference<DocTree>& nd)
		{m_chld.push_back( nd);}

	typedef std::list<strus::Reference<DocTree> >::const_iterator chld_iterator;
	typedef std::list<Attribute>::const_iterator attr_iterator;

private:
	std::string m_name;				///... name of the tag or empty for a content node (content of a node that has sub nodes)
	std::string m_value;				///... direct content of the node if without sub nodes
	std::list<Attribute> m_attr;			///... list of attributes
	std::list<strus::Reference<DocTree> > m_chld;	///... list of sub nodes
};

/// \brief Pointer to document tree or a node of a tree
typedef strus::Reference<DocTree> DocTreeRef;

/// \brief Get a document XML as tree structure without meta info
/// \return the document content as tree
/// \param[in] encoding character set encoding of the document to parse
/// \param[in] src pointer to source of the document to parse
/// \param[in] srcsize size of the document to parse in bytes
/// \param[in,out] errorhnd errorbuffer interface where to report errors
/// \note the resulting document tree dismisses XML header elements and doctype attributes
DocTree* createDocTree_xml( const char* encoding, const char* src, std::size_t srcsize, ErrorBufferInterface* errorhnd);

/// \brief Output a document XML from a tree structure
/// \param[out] where to print the output to
/// \param[in] encoding character set encoding of the document to print
/// \param[in] tree the document to print
/// \param[in,out] errorhnd errorbuffer interface where to report errors
/// \return true on success, false on error
bool printDocTree_xml( std::ostream& out, const char* encoding, const DocTree& tree, ErrorBufferInterface* errorhnd);

}//namespace
#endif

