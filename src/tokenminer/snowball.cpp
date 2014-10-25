/*
---------------------------------------------------------------------
    The C++ library strus implements basic operations to build
    a search engine for structured search on unstructured data.

    Copyright (C) 2013,2014 Patrick Frey

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------

	The latest version of strus can be found at 'http://github.com/patrickfrey/strus'
	For documentation see 'http://patrickfrey.github.com/strus'

--------------------------------------------------------------------
*/
#include "snowball.hpp"

using namespace strus;
#include "danish_stem.h"
#include "dutch_stem.h"
#include "english_stem.h"
#include "finnish_stem.h"
#include "french_stem.h"
#include "german_stem.h"
#include "italian_stem.h"
#include "norwegian_stem.h"
#include "portuguese_stem.h"
#include "spanish_stem.h"
#include "swedish_stem.h"


template <class snowball_stemmer>
class StemNormalizer
	:public NormalizerInterface
{
public:
	virtual std::string normalize( const char* src, std::size_t srcsize) const
	{
		std::string rt = std::string( src, srcsize);
		snowball_stemmer()( rt);
		return rt;
	}
};

static const StemNormalizer<stemming::danish_stem<> > stemNormalizer_dk;
static const StemNormalizer<stemming::dutch_stem<> > stemNormalizer_nl;
static const StemNormalizer<stemming::english_stem<> > stemNormalizer_en;
static const StemNormalizer<stemming::finnish_stem<> > stemNormalizer_fi;
static const StemNormalizer<stemming::french_stem<> > stemNormalizer_fr;
static const StemNormalizer<stemming::german_stem<> > stemNormalizer_de;
static const StemNormalizer<stemming::italian_stem<> > stemNormalizer_it;
static const StemNormalizer<stemming::norwegian_stem<> > stemNormalizer_no;
static const StemNormalizer<stemming::portuguese_stem<> > stemNormalizer_pt;
static const StemNormalizer<stemming::spanish_stem<> > stemNormalizer_es;
static const StemNormalizer<stemming::swedish_stem<> > stemNormalizer_se;

const NormalizerInterface* strus::snowball_stemmer_de()	{return &stemNormalizer_de;}
const NormalizerInterface* strus::snowball_stemmer_dk()	{return &stemNormalizer_de;}
const NormalizerInterface* strus::snowball_stemmer_nl()	{return &stemNormalizer_de;}
const NormalizerInterface* strus::snowball_stemmer_en()	{return &stemNormalizer_de;}
const NormalizerInterface* strus::snowball_stemmer_fi()	{return &stemNormalizer_de;}
const NormalizerInterface* strus::snowball_stemmer_fr()	{return &stemNormalizer_de;}
const NormalizerInterface* strus::snowball_stemmer_it()	{return &stemNormalizer_de;}
const NormalizerInterface* strus::snowball_stemmer_no()	{return &stemNormalizer_de;}
const NormalizerInterface* strus::snowball_stemmer_pt()	{return &stemNormalizer_de;}
const NormalizerInterface* strus::snowball_stemmer_es()	{return &stemNormalizer_de;}
const NormalizerInterface* strus::snowball_stemmer_se()	{return &stemNormalizer_de;}

