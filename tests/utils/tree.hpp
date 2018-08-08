/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_ANALYZER_TEST_TREE_HPP_INCLUDED
#define _STRUS_ANALYZER_TEST_TREE_HPP_INCLUDED
#include "strus/base/stdint.h"
#include <iostream>
#include <sstream>

namespace strus {
namespace test {

template <typename ITEM>
struct TreeNode
{
	ITEM item;
	struct TreeNode* next;
	struct TreeNode* chld;

	TreeNode( const ITEM& item_)
		:item(item_),next(0),chld(0){}
	TreeNode( const TreeNode& o)
		:item(o.item)
		,next( o.next ? new TreeNode( *o.next) : 0)
		,chld( o.chld ? new TreeNode( *o.chld) : 0){}
	~TreeNode()
	{
		if (next) delete next;
		if (chld) delete chld;
	}

	void addChild( const TreeNode& nd)
	{
		if (chld)
		{
			chld->addSibling( nd);
		}
		else
		{
			chld = new TreeNode( nd);
		}
	}

	void addSibling( const TreeNode& nd)
	{
		TreeNode* np = this;
		for (; np->next; np = np->next){}
		np->next = new TreeNode( nd);
	}

	void print( std::ostream& out)
	{
		print( out, 0);
	}

	int size() const
	{
		TreeNode* np = this;
		int ii = 1;
		for (; np->next; np=np->next,++ii){}
		return ii;
	}

private:
	void print( std::ostream& out, std::size_t indent)
	{
		std::string indentstr( indent*2, ' ');
		TreeNode* np = this;
		do
		{
			out << indentstr << np->item << std::endl;
			if (np->chld) np->chld->print( out, indent+2);
			np = np->next;
		} while (np);
	}
};

}}//namespace
#endif


