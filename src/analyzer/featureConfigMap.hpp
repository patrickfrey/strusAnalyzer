/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _STRUS_DOCUMENT_ANALYZER_FEATURE_CONFIG_MAP_HPP_INCLUDED
#define _STRUS_DOCUMENT_ANALYZER_FEATURE_CONFIG_MAP_HPP_INCLUDED
#include "featureConfig.hpp"
#include <vector>
#include <string>

namespace strus
{

enum {
	MaxSegmenterId=(1<<30),
	SubDocumentEnd=(1<<24),
	OfsSubDocument=SubDocumentEnd+1,
	MaxNofSubDocuments=(MaxSegmenterId - OfsSubDocument - 1),

	OfsPreProcPatternMatchSegment=(1<<22),
	MaxNofPreProcPatternMatchSegments=(SubDocumentEnd-OfsPreProcPatternMatchSegment-1),

	OfsPostProcPatternMatchSegment=(1<<20),
	MaxNofPostProcPatternMatchSegments=(OfsPreProcPatternMatchSegment-OfsPostProcPatternMatchSegment-1),

	OfsPatternMatchSegment=OfsPostProcPatternMatchSegment,

	EndOfFeatures=OfsPatternMatchSegment-1,
	MaxNofFeatures=EndOfFeatures-1
};

/// \brief Set of configured features
class FeatureConfigMap
{
public:
	FeatureConfigMap()
		:m_ar(){}
	~FeatureConfigMap(){}

	unsigned int defineFeature(
		FeatureClass featureClass,
		const std::string& name,
		TokenizerFunctionInstanceInterface* tokenizer,
		const std::vector<NormalizerFunctionInstanceInterface*>& normalizers,
		const analyzer::FeatureOptions& options);

	const FeatureConfig& featureConfig( int featidx) const;

	typedef std::vector<FeatureConfig>::const_iterator const_iterator;
	const_iterator begin() const		{return m_ar.begin();}
	const_iterator end() const		{return m_ar.end();}

private:
	std::vector<FeatureConfig> m_ar;
};

}//namespace
#endif


