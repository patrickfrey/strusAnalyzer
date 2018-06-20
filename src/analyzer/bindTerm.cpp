/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "bindTerm.hpp"
#include <algorithm>
#include <set>

using namespace strus;

struct TermPrio
{
	int seg;
	int ofs;
	int endofs;
	int priority;
	int idx;

	TermPrio( int seg_, int ofs_, int endofs_, int priority_, int idx_)
		:seg(seg_),ofs(ofs_),endofs(endofs_),priority(priority_),idx(idx_){}
#if __cplusplus >= 201103L
	TermPrio( TermPrio&& ) = default;
	TermPrio( const TermPrio& ) = default;
	TermPrio& operator= ( TermPrio&& ) = default;
	TermPrio& operator= ( const TermPrio& ) = default;
#else
	TermPrio( const TermPrio& o)
		:seg(o.seg),ofs(o.ofs),endofs(o.endofs),priority(o.priority),idx(o.idx){}
#endif

	bool operator<( const TermPrio& o) const
	{
		if (seg < o.seg) return true;
		if (seg > o.seg) return false;
		if (ofs < o.ofs) return true;
		if (ofs > o.ofs) return false;
		if (priority > o.priority) return true;
		if (priority < o.priority) return false;
		if (endofs < o.endofs) return true;
		if (endofs > o.endofs) return false;
		return idx < o.idx;
	}
};

void BindTerm::eliminateCoveredElements( std::vector<BindTerm>& terms)
{
	std::set<TermPrio> prioset;
	std::vector<BindTerm>::const_iterator ti = terms.begin(), te = terms.end();
	if (ti == te) return;
	int min_priority = ti->priority();
	bool has_diff = false;
	for (++ti; ti != te; ++ti)
	{
		if (ti->priority() != min_priority)
		{
			has_diff = true;
			if (ti->priority() < min_priority)
			{
				min_priority = ti->priority();
				for (++ti; ti != te; ++ti)
				{
					if (ti->priority() < min_priority)
					{
						min_priority = ti->priority();
					}
				}
			}
			break;
		}
	}
	if (!has_diff) return;
	int tidx = 0;
	for (ti = terms.begin(); ti != te; ++ti,++tidx)
	{
		prioset.insert( TermPrio( ti->seg(), ti->ofs(), ti->endofs(), ti->priority(), tidx));
	}
	std::set<int> eliminated;
	std::set<TermPrio>::iterator pi = prioset.begin(), pe = prioset.end();
	for (; pi != pe; ++pi)
	{
		if (pi->priority > min_priority)
		{
			std::set<TermPrio>::iterator pn = pi;
			for (++pn; pn != pe; ++pn)
			{
				if (pn->seg != pi->seg) break;
				if (pi->ofs > pn->endofs) break;
				if (pi->endofs >= pn->endofs && pi->priority > pn->priority)
				{
					eliminated.insert( pn->idx);
				}
			}
		}
	}
	if (eliminated.empty()) return;

	std::set<int>::reverse_iterator ei = eliminated.rbegin(), ee = eliminated.rend();
	for (; ei != ee; ++ei)
	{
		if ((std::size_t)*ei != terms.size()-1)
		{
			std::swap( terms[ *ei], terms.back());
		}
		terms.pop_back();
	}
	std::sort( terms.begin(), terms.end());
}


