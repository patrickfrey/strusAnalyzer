/*
 * Copyright (c) 2018 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Standard program loader class of the analyzer (load program in a domain specific language)
#ifndef _STRUS_ANALYZER_PROGRAM_LOADER_STD_HPP_INCLUDED
#define _STRUS_ANALYZER_PROGRAM_LOADER_STD_HPP_INCLUDED
#include "strus/analyzer/documentClass.hpp"
#include <string>
#include <vector>

namespace strus {

/// \brief Forward declaration
class ErrorBufferInterface;
/// \brief Forward declaration
class DocumentAnalyzerInterface;
/// \brief Forward declaration
class TextProcessorInterface;

/* \brief Load a document analyzer program defined with a domain specific language
 * \param[in] analyzer analyzer to fill with the definitions in the source file
 * \param[in] textproc textprocessor interface providing the builders for the objects defined
 * \param[in] source the source to parse (content and NOT a file name)
 * \param[in] allowIncludes true, if include directives are allowed at the beginning of the content to parse
 * \note Include directives have the syntax:
            IncludeDecl      :- "#include" FileName
 * \param[in,out] errorhnd interface for reporting errors and exceptions occurred
 * \note Comments in the domain specific language are end of line comments starting with '#'
 * \remark the include section must not have comments (includes must be declared before the first include)
 * \note The domain specific language has the following grammar:
 
	AnalyzerProgram      : { ContentSection | DocumentSection | AggregatorSection | PatternMatchSection | FeatureSection }
	ContentSection       :- "[" "Content" "]" { ContentDecl ";" }
	ContentDecl          :- DocumentClass ContentSelection
	DocumentClass        :- # String: MIME Content Type Declaration
	ContentSelection     :- # Abbrev XPath expression: Selection of the section with a different segmenter

	DocumentSection      :- "[" "Document" "]" { DocumentDecl ";" }
	DocumentDecl         :- DocumentType "=" DocumentSelection
	DocumentType         :- # Identifier: Sub document type name
	DocumentSelection    :- # Abbrev XPath expression: Selection of the content that embodies a sub document in a multipart document

	AggregatorSection    :- "[" "Aggregator" "]" { AggregatorDecl ";" }
	AggregatorDecl       :- AggregatorFuncName "=" { FunctionDecl ";" }
	AggregatorFuncName   :- # Identifier: Name of the metadata feature where the result of the aggregator is stored
	FunctionDecl         :- FunctionName [ "(" FunctionParamList ")" ]
	FunctionName         :- # Identifier: Name of the function (function type depending on context) defined in the textprocessor
	FunctionParamList    :- FunctionParam { "," FunctionParam }
	FunctionParam        :- Atomic value representing a parameter passed to the function

	PatternMatchSection  :- "[" "PatternMatch" [ PatternMatchModuleId ] "]" { [ PrePatternMatchDecl | PostPatternMatchDecl } ";" }
	PatternMatchModuleId :- # Identifier: Name of the module used for pattern matching, use standard module if not defined
	PrePatternMatchDecl  :- PatternFeatureType "=" "{" SelectExprList "}" PatternFile
	PatternFeatureType   :- # Identifier: Name of the feature that is produced by the pattern matcher
	SelectExprList       :- # Abbrev XPath expression: Selection of the content that is used as input for the pre processing pattern matcher
	PatternFile          :- # File name: Name of the file with the pattern matcher program to load
	PostPatternMatchDecl :- PatternFeatureType "=" PatternFile

	FeatureSection       :- "[" FeatureName "]" { FeatureDecl ";" }
	FeatureClassName     :- # One of "ForwardIndex","SearchIndex","Attribute","MetaData","PatternLexem" defining the class of feature created
	FeatureDecl          :- FeatureType "=" NormalizerDefList TokenizerDef SelectionDef
	FeatureDecl          :- FeatureType "<-" PatternName NormalizerDefList
	FeatureType          :- # Identifier: Type name of the feature produced
	PatternName          :- # Identifier: Name of the pattern that is used as input passed to a sequence of normalizers to create the feature 
	NormalizerDefList    :- NormalizerDef { ":" NormalizerDef }
	NormalizerDef        :- FunctionDecl
	TokenizerDef         :- FunctionDecl
	SelectionDef         :- # Abbrev XPath expression: Selection of the content that is used as input for the building of this feature
*/
bool loadDocumentAnalyzerProgram(
		DocumentAnalyzerInterface* analyzer,
		const TextProcessorInterface* textproc,
		const std::string& source,
		bool allowIncludes,
		ErrorBufferInterface* errorhnd);

}//namespace
#endif



