/*
 * Copyright (c) 2016 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Instance interface for defining a mapping of terms of the document analysis outout as lexems used as basic entities by pattern matching
/// \file "patternTermFeederInstanceInterface.hpp"
#ifndef _STRUS_ANALYZER_PATTERN_TERM_FEEDER_INSTANCE_INTERFACE_HPP_INCLUDED
#define _STRUS_ANALYZER_PATTERN_TERM_FEEDER_INSTANCE_INTERFACE_HPP_INCLUDED
#include "strus/analyzer/patternLexem.hpp"
#include "strus/analyzer/patternMatcherResult.hpp"
#include "strus/structView.hpp"
#include <string>

namespace strus
{
/// \brief Instance interface for defining a mapping of terms of the document analysis outout as lexems used as basic entities by pattern matching
class PatternTermFeederInstanceInterface
{
public:
	/// \brief Destructor
	virtual ~PatternTermFeederInstanceInterface(){}

	/// \brief Define a term type
	/// \param[in] id identifier given to the lexem assiciated with this term type
	/// \param[in] type term type string
	/// \remark Because 0 is reserved for undefined values, the id parameter must not be 0
	/// \remark For performance it may be significantly better to define rules without resultIndex selection.
	virtual void defineLexem(
			unsigned int id,
			const std::string& type)=0;

	/// \brief Define a symbol, an instance of a basic lexem, that gets a different id than the basic lexem
	/// \param[in] id identifier given to the result substring, 0 if the result term is not appearing in the output
	/// \param[in] lexemid identifier of the basic lexem this symbol belongs to
	/// \param[in] name name (value string) of the symbol
	virtual void defineSymbol(
			unsigned int id,
			unsigned int lexemid,
			const std::string& name)=0;

	/// \brief Get the value of a defined lexem by type
	/// \param[in] type type name of the lexem
	/// \return the lexem identifier or 0, if not defined
	virtual unsigned int getLexem(
			const std::string& type) const=0;

	/// \brief Get the list of lexem types defined
	/// \return the list of lexem types
	virtual std::vector<std::string> lexemTypes() const=0;

	/// \brief Get the identifier of a defined symbol
	/// \param[in] lexemid identifier of the basic lexem this symbol belongs to
	/// \param[in] name name (value string) of the symbol
	/// \return the symbol identifier or 0, if not defined
	/// \remark this function is needed because symbols are most likely implicitely defined on demand by reference
	virtual unsigned int getSymbol(
			unsigned int lexemid,
			const std::string& name) const=0;

	/// \brief Get the name of the function
	/// \return the identifier
	virtual const char* name() const=0;

	/// \brief Return a structure with all definitions for introspection
	/// \return the structure with all definitions for introspection
	virtual StructView view() const=0;
};

} //namespace
#endif

