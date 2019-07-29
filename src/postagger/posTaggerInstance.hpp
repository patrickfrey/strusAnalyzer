/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Implementation of interface to define a POS tagger instance for creating the input for POS tagging to build the data and to create to context for tagging with the data build from the POS tagging output
/// \file posTaggerInstance.hpp
#ifndef _STRUS_ANALYZER_POS_TAGGER_INSTANCE_IMPLEMENTATION_HPP_INCLUDED
#define _STRUS_ANALYZER_POS_TAGGER_INSTANCE_IMPLEMENTATION_HPP_INCLUDED
#include "strus/posTaggerInstanceInterface.hpp"
#include "strus/analyzer/segmenterOptions.hpp"
#include <string>

/// \brief strus toplevel namespace
namespace strus
{
/// \brief Forward declaration
class SegmenterInterface;
/// \brief Forward declaration
class SegmenterInstanceInterface;
/// \brief Forward declaration
class TokenMarkupInstanceInterface;
/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DebugTraceContextInterface;
/// \brief Forward declaration
class PosTaggerDataInterface;

/// \brief Implementation of the POS tagger instance for creating the input for POS tagging to build the data and to create to context for tagging with the data build from the POS tagging output
class PosTaggerInstance
	:public PosTaggerInstanceInterface
{
public:
	/// \param[in] segmenter_ segmenter instance to use (passed with ownership)
	/// \param[in] errorhnd_ error buffer interface
	PosTaggerInstance( const SegmenterInterface* segmenter_, const analyzer::SegmenterOptions& opts, ErrorBufferInterface* errorhnd_);
	virtual ~PosTaggerInstance();

	virtual void addContentExpression( const std::string& expression);
	virtual void addPosTaggerInputPunctuation( const std::string& expression, const std::string& value, int priority);

	virtual std::string getPosTaggerInput( const analyzer::DocumentClass& dclass, const std::string& content) const;

	virtual std::string markupDocument( const PosTaggerDataInterface* data, int docno, const analyzer::DocumentClass& dclass, const std::string& content) const;

private:
	void cleanup();

private:
	class PunctuationDef
	{
	public:
		PunctuationDef( const PunctuationDef& o)
			:m_value(o.m_value),m_priority(o.m_priority){}
		PunctuationDef( const std::string& value_, int priority_)
			:m_value(value_),m_priority(priority_){}

		const std::string& value() const	{return m_value;}
		int priority() const			{return m_priority;}

		bool operator==( const PunctuationDef& o) const
		{
			return m_value == o.m_value && m_priority == o.m_priority;
		}
	private:
		std::string m_value;
		int m_priority;
	};

private:
	ErrorBufferInterface* m_errorhnd;
	DebugTraceContextInterface* m_debugtrace;
	SegmenterInstanceInterface* m_segmenter;
	TokenMarkupInstanceInterface* m_markup;
	std::vector<PunctuationDef> m_punctar;
};

}//namespace
#endif

