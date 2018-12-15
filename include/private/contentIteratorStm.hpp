/*
* Copyright (c) 2018 Patrick P. Frey
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/// \brief Helper class for content iterators
/// \file contentIteratorStm.hpp
#ifndef _STRUS_ANALYZER_CONTENT_ITERATOR_STATEMACHINE_HPP_INCLUDED
#define _STRUS_ANALYZER_CONTENT_ITERATOR_STATEMACHINE_HPP_INCLUDED
#include <string>
#include <vector>
#include <set>
#include "textwolf/xmlscanner.hpp"
#include "private/internationalization.hpp"

/// \brief strus toplevel namespace
namespace strus {

/// \brief Implementation of content statistics
class ContentIteratorStm
{
public:
	class Path
	{
	public:
		Path()
			:m_attribpos(0){}
		Path( const Path& o)
			:m_ar(o.m_ar),m_stk(o.m_stk),m_attribpos(o.m_attribpos){}

		void open( const char* name, std::size_t namelen)
		{
			m_ar.resize( m_attribpos);
			m_stk.push_back( m_ar.size());
			m_ar.push_back( '/');
			m_ar.append( name, namelen);
			m_attribpos = m_ar.size();
		}
		void close()
		{
			m_ar.resize( m_stk.back());
			m_stk.pop_back();
			m_attribpos = m_ar.size();
		}
		void selectAttribute( const char* name, std::size_t namelen)
		{
			m_ar.resize( m_attribpos);
			m_ar.push_back('@');
			m_ar.append( name, namelen);
		}
		void selectValue()
		{
			if (back() != ')')
			{
				m_ar.append( "()");
			}
		}
		char back() const
		{
			if (m_ar.empty()) return '\0';
			return m_ar[m_ar.size()-1];
		}
		void attributeCondName( const char* name, std::size_t namelen)
		{
			if (back() == ']' || back() == '=')
			{
				m_ar[m_ar.size()-1] = ',';
			}
			else
			{
				m_ar.push_back('[');
			}
			m_ar.push_back('@');
			m_ar.append( name, namelen);
			m_ar.push_back('=');
		}
		void attributeCondValue( const char* value, std::size_t valuelen)
		{
			m_ar.push_back('"');
			m_ar.append( encodeAttributeValue( value, valuelen));
			m_ar.push_back('"');
			m_ar.push_back(']');
		}
		const std::string& operator()() const
		{
			return m_ar;
		}

	private:
		static std::string encodeAttributeValue( const char* value, std::size_t valuelen)
		{
			static const char* lit = "\"\n\r\t\b\f\v";
			static const char* trs = "\"nrtbfv";
			std::string rt;
			char const* vi = value;
			const char* ve = vi + valuelen;
			for (; vi != ve; ++vi)
			{
				const char* li = std::strchr( lit, *vi);
				if (li)
				{
					rt.push_back('\\');
					rt.push_back( trs[ li-lit]);
				}
				else
				{
					rt.push_back( *vi);
				}
			}
			return rt;
		}
	private:
		std::string m_ar;
		std::vector<int> m_stk;
		int m_attribpos;
	};

	explicit ContentIteratorStm( const std::set<std::string>* attributes_)
		:m_attributes(attributes_),m_contentpath(),m_attrpath(),m_selectpath(),m_state(StateAttribute)
	{}

	void openTag( const char* name, std::size_t namelen)
	{
		m_attrpath.open( name, namelen);
		m_contentpath = m_attrpath;
		m_state = StateAttribute;
	}

	void closeTag()
	{
		m_contentpath.close();
		m_attrpath.close();
		m_state = StateAttribute;
	}

	void attributeName( const char* name, std::size_t namelen)
	{
		m_contentpath.selectAttribute( name, namelen);
		if (m_attributes->find( std::string( name, namelen)) != m_attributes->end())
		{
			m_attrpath.attributeCondName( name, namelen);
			m_state = StateContent;
		}
	}
	void attributeValue( const char* value, std::size_t valuelen)
	{
		if (m_state == StateContent)
		{
			m_attrpath.attributeCondValue( value, valuelen);
			m_state = StateAttribute;
		}
	}
	void selectValue()
	{
		m_attrpath.selectValue();
	}
	bool textwolfItem(
			const textwolf::XMLScannerBase::ElementType& itemtype, const char* itemstr, std::size_t itemsize,
			const char*& expression, std::size_t& expressionsize,
			const char*& segment, std::size_t& segmentsize)
	{
		switch (itemtype)
		{
			case textwolf::XMLScannerBase::None:
			case textwolf::XMLScannerBase::ErrorOccurred:
			case textwolf::XMLScannerBase::HeaderStart:
			case textwolf::XMLScannerBase::HeaderAttribName:
			case textwolf::XMLScannerBase::HeaderAttribValue:
			case textwolf::XMLScannerBase::HeaderEnd:
			case textwolf::XMLScannerBase::DocAttribValue:
			case textwolf::XMLScannerBase::DocAttribEnd:
				break;
			case textwolf::XMLScannerBase::TagAttribName:
				this->attributeName( itemstr, itemsize);
				break;
			case textwolf::XMLScannerBase::TagAttribValue:
				m_selectpath = std::string( m_contentpath());
				this->attributeValue( itemstr, itemsize);
				expression = m_selectpath.c_str();
				expressionsize = m_selectpath.size();
				segment = itemstr;
				segmentsize = itemsize;
				return true;
			case textwolf::XMLScannerBase::OpenTag:
				this->openTag( itemstr, itemsize);
				break;
			case textwolf::XMLScannerBase::CloseTag:
			case textwolf::XMLScannerBase::CloseTagIm:
				this->closeTag();
				break;
			case textwolf::XMLScannerBase::Content:
				this->selectValue();
				expression = m_attrpath().c_str();
				expressionsize = m_attrpath().size();
				segment = itemstr;
				segmentsize = itemsize;
				return true;
			case textwolf::XMLScannerBase::Exit:
				return false;
		}
		return false;
	}

	const std::string& path() const
	{
		switch (m_state)
		{
			case StateContent: return m_contentpath();
			case StateAttribute: return m_attrpath();
		}
		return m_contentpath();
	}

private:
	enum State {
		StateContent,
		StateAttribute
	};

private:
	const std::set<std::string>* m_attributes;
	Path m_contentpath;
	Path m_attrpath;
	std::string m_selectpath;
	State m_state;
};

}//namespace
#endif

