/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_SEGMENTER_XPATH_AUTOMATON_TEXTWOLF_HPP_INCLUDED
#define _STRUS_SEGMENTER_XPATH_AUTOMATON_TEXTWOLF_HPP_INCLUDED
#include "textwolf/xmlpathselect.hpp"
#include "textwolf/xmlpathautomaton.hpp"
#include "textwolf/xmlpathautomatonparse.hpp"
#include "textwolf/xmlscanner.hpp"
#include "textwolf/charset.hpp"
#include "textwolf/sourceiterator.hpp"
#include <cstdlib>
#include <setjmp.h>

namespace strus
{

class XPathAutomatonContext
{
public:
	typedef textwolf::XMLPathSelectAutomaton<> Automaton;

public:
	explicit XPathAutomatonContext( const Automaton* automaton_);
	~XPathAutomatonContext(){}

	void putElement(
		const textwolf::XMLScannerBase::ElementType& elemtype,
		const char* elem_,
		std::size_t elemsize_);

	bool getNext( int& id);

private:
	typedef textwolf::XMLPathSelect<
			textwolf::charset::UTF8
		> XMLPathSelect;

	const Automaton* m_automaton;
	XMLPathSelect m_pathselect;
	XMLPathSelect::iterator m_selitr;
	XMLPathSelect::iterator m_selend;
};


class XPathAutomaton
{
public:
	XPathAutomaton(){}
	~XPathAutomaton(){}

	void defineSelectorExpression( int id, const std::string& expression);
	void defineSubSection( int startId, int endId, const std::string& expression);

	XPathAutomatonContext createContext() const;

private:
	void addExpression( int id, const std::string& expression);

private:
	typedef textwolf::XMLPathSelectAutomatonParser<> Automaton;
	Automaton m_automaton;
};


} // namespace
#endif

