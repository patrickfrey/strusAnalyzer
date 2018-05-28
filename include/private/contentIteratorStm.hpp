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
#include "textwolf/xmlscanner.hpp"
#include "private/internationalization.hpp"

/// \brief strus toplevel namespace
namespace strus {

/// \brief Implementation of content statistics
class ContentIteratorStm
{
public:
	ContentIteratorStm()
		:m_contentpath(),m_attrpath(),m_selectpath(),m_stk(),m_attribpos(0),m_state(StateContent)
	{}

	void openTag( const char* name, std::size_t namelen)
	{
		m_stk.push_back( m_contentpath.size());
		m_contentpath.push_back( '/');
		m_contentpath.append( name, namelen);
		m_attrpath.push_back( '/');
		m_attrpath.append( name, namelen);
		m_attribpos = m_attrpath.size();
		m_state = StateContent;
	}

	void closeTag()
	{
		m_contentpath.resize( m_stk.back());
		m_attrpath.resize( m_stk.back());
		m_attribpos = m_attrpath.size();
		m_stk.pop_back();
		m_state = StateContent;
	}

	void attributeName( const char* name, std::size_t namelen)
	{
		if (m_stk.empty())
		{
			m_attrpath.resize( m_attribpos);
			m_attrpath.push_back('@');
			m_attrpath.append( name, namelen);
		}
		else
		{
			if (m_contentpath[ m_contentpath.size()-1] == ']' || m_contentpath[ m_contentpath.size()-1] == '=')
			{
				m_contentpath[ m_contentpath.size()-1] = ',';
			}
			else
			{
				m_contentpath.push_back('[');
			}
			m_contentpath.push_back('@');
			m_contentpath.append( name, namelen);
			m_contentpath.push_back('=');
		}
		m_state = StateAttribute;
	}

	void attributeValue( const char* value, std::size_t valuelen)
	{
		m_contentpath.push_back('\"');
		m_contentpath.append( encodeAttributeValue( value, valuelen));
		m_contentpath.push_back('\"');
		m_contentpath.push_back(']');
		m_state = StateContent;
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
				m_selectpath = std::string( this->path());
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
				expression = this->path().c_str();
				expressionsize = this->path().size();
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
			case StateContent: return m_contentpath;
			case StateAttribute: return m_attrpath;
		}
		return m_contentpath;
	}

private:
	enum State {
		StateContent,
		StateAttribute
	};

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
	std::string m_contentpath;
	std::string m_attrpath;
	std::string m_selectpath;
	std::vector<int> m_stk;
	int m_attribpos;
	State m_state;
};

}//namespace
#endif

