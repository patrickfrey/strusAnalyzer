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
class DocumentAnalyzerInstanceInterface;
/// \brief Forward declaration
class QueryAnalyzerInstanceInterface;
/// \brief Forward declaration
class TextProcessorInterface;
/// \brief Forward declaration
class DocumentAnalyzerMapInterface;

/// \brief parse the document class from source
/// \param[in] src document class definition as string
/// \param[in,out] errorhnd interface for reporting errors and exceptions occurred
/// \return the document class structure
analyzer::DocumentClass parseDocumentClass(
		const std::string& src,
		ErrorBufferInterface* errorhnd);

/* \brief Load a document analyzer program defined with a domain specific language
 * \param[in] analyzer analyzer to fill with the definitions in the source
 * \param[in] textproc textprocessor interface providing the builders for the objects defined
 * \param[in] source the source to parse (content and NOT a file name) in a domain specific language
 * \param[in] allowIncludes true, if include directives are allowed at the beginning of the content to parse
 * \note Include directives have the syntax:
            IncludeDecl      :- "#include" FileName
 * \param[in,out] errorhnd interface for reporting errors and exceptions occurred
 * \note Comments in the domain specific language are end of line comments starting with '#'
 * \remark the include section must not have comments (includes must be declared before the first include)
 * \note The domain specific language has the following grammar:
 
	AnalyzerProgram      : { ContentSection | DocumentSection | AggregatorSection | FeatureSection }
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

	FeatureSection       :- "[" FeatureClassName "]" { FeatureDecl ";" }
	FeatureClassName     :- # One of "ForwardIndex","SearchIndex","Attribute","MetaData" defining the class of feature created
	FeatureDecl          :- FeatureType "=" NormalizerDefList TokenizerDef SelectionDef
	FeatureType          :- # Identifier: Type name of the feature produced
	NormalizerDefList    :- NormalizerDef { ":" NormalizerDef }
	NormalizerDef        :- FunctionDecl
	TokenizerDef         :- FunctionDecl
	SelectionDef         :- # Abbrev XPath expression: Selection of the content that is used as input for the building of this feature

	FieldSection         :- "[" "Field" "]" { FieldDecl ";" }
	FieldDecl            :- FieldTypeName "=" ScopeSelectionDef ContentDef [ IdDef ]
	FieldTypeName        :- # Identifier: Type name of the field that is used to address it when declaring structures
	ScopeSelectionDef    :- Abbrev XPath expression: Selection of the content that covers all elements of the field and serves as scope and key for relating fields to build structures. It serves also ase base path for the definition of content and optionally id.
	ContentDef           :- Abbrev XPath expression relative to ScopeSelectionDef. Identifies the content of the field.
	KeyDef               :- Abbrev XPath expression relative to ScopeSelectionDef. Optional. Identifies the key to relate fields in structures if the scope is not enough (e.g. table rows and columns).

	StructureSection     :- "[" "Structure" "]" { StructureDecl ";" }
	StructureDecl        :- StructureName "=" FieldNameHeader FieldNameContent StructureType
	StructureName        :- # Identifier: Class name of the structure that is used to address it
	FieldNameHeader      :- # Identifier: Class name of the field that serves as header of the structure
	FieldNameContent     :- # Identifier: Class name of the field that serves as content of the structure
	StructureType        :- # One of "hierarchical" (the content encloses the header or wise versa), "heading" (the header preceedes content until a new "heading" is declared, "footer" (the header succeeds the contents preeceeding it without other "footer" defined)
*/
bool loadDocumentAnalyzerProgramSource(
		DocumentAnalyzerInstanceInterface* analyzer,
		const TextProcessorInterface* textproc,
		const std::string& source,
		bool allowIncludes,
		ErrorBufferInterface* errorhnd);

/// \brief Load a document analyzer program defined with a domain specific language
/// \param[in] analyzer analyzer to fill with the definitions in the source
/// \param[in] textproc textprocessor interface providing the builders for the objects defined
/// \param[in] filename the name of the file to parse in a domain specific language (see the syntax description for 'loadDocumentAnalyzerProgramSource')
/// \param[in,out] errorhnd interface for reporting errors and exceptions occurred
/// \return true if yes, false if not or in case of an error
bool loadDocumentAnalyzerProgramFile(
		DocumentAnalyzerInstanceInterface* analyzer,
		const TextProcessorInterface* textproc,
		const std::string& filename,
		ErrorBufferInterface* errorhnd);

/// \brief Test if a content is a document analyzer program source
/// \param[in] textproc text processor interface to determine the path of the filename
/// \param[in] filename name of the file to load
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
/// \return true if yes, false if not or in case of an error
bool isDocumentAnalyzerProgramFile(
		const TextProcessorInterface* textproc,
		const std::string& filename,
		ErrorBufferInterface* errorhnd);

/// \brief Test if a content is a document analyzer program source
/// \param[in] textproc text processor interface to access functions needed
/// \param[in] source source with definitions
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
/// \return true if yes, false if not or in case of an error
bool isDocumentAnalyzerProgramSource(
		const std::string& source,
		ErrorBufferInterface* errorhnd);

/// \brief Load a document analyzer map defined with a domain specific language
/// \param[in] analyzermap analyzer map to fill with the definitions in the source
/// \param[in] textproc textprocessor interface providing the builders for the objects defined
/// \param[in] source the source to parse (content and NOT a file name) in a domain specific language
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
/*
*   \note Comments in the domain specific language are end of line comments starting with '#'
*   \note The domain specific language has the following grammar:
*        AnalyzerMap           : {MapDefinition ';'}
*        MapDefinition         : { "analyze" DocumentClass "program" AnalyzerProgramName
*        DocumentClass         :- # String: MIME Content Type Declaration
*        AnalyzerProgramName   :  # String or path identifying the program file to load for this document class
*/
bool loadDocumentAnalyzerMapSource(
		DocumentAnalyzerMapInterface* analyzermap,
		const TextProcessorInterface* textproc,
		const std::string& source,
		ErrorBufferInterface* errorhnd);

/// \brief Load a document analyzer map defined with a domain specific language
/// \param[in] analyzermap analyzer map to fill with the definitions in the source
/// \param[in] textproc textprocessor interface providing the builders for the objects defined
/// \param[in] filename the name of the file to parse in a domain specific language (see the syntax description for 'loadDocumentAnalyzerProgramSource')
/// \param[in,out] errorhnd buffer for reporting errors (exceptions)
bool loadDocumentAnalyzerMapFile(
		DocumentAnalyzerMapInterface* analyzermap,
		const TextProcessorInterface* textproc,
		const std::string& filename,
		ErrorBufferInterface* errorhnd);

/* \brief Load a query analyzer program defined with a domain specific language
 * \param[in] analyzer analyzer to fill with the definitions in the source
 * \param[in] textproc textprocessor interface providing the builders for the objects defined
 * \param[in] source the source to parse (content and NOT a file name) in a domain specific language
 * \param[in] allowIncludes true, if include directives are allowed at the beginning of the content to parse
 * \note Include directives have the syntax:
            IncludeDecl      :- "#include" FileName
 * \param[in,out] errorhnd interface for reporting errors and exceptions occurred
 * \note Comments in the domain specific language are end of line comments starting with '#'
 * \remark the include section must not have comments (includes must be declared before the first include)
 * \note The domain specific language has the following grammar:
 
	AnalyzerProgram      : { PrioritySection | QueryElementSection }

	PrioritySection      :- "[" "Priority" "]" { PriorityDecl ";" }
	PriorityDecl         :- FeatureType "=" Priority ";"
	Priority             :- # Integer number

	QueryElementSection  :- "[" ElementClassName "]" { FeatureDecl ";" }
	ElementClassName     :- # One of "Element" defining the class of the element created
	FeatureDecl          :- FeatureType "=" NormalizerDefList TokenizerDef FieldName
	FeatureType          :- # Identifier: Type name of the feature produced
	NormalizerDefList    :- NormalizerDef { ":" NormalizerDef }
	NormalizerDef        :- FunctionDecl
	TokenizerDef         :- FunctionDecl
	FieldName            :- # Identifier: Name of the query field
*/
bool loadQueryAnalyzerProgramSource(
		QueryAnalyzerInstanceInterface* analyzer,
		const TextProcessorInterface* textproc,
		const std::string& source,
		bool allowIncludes,
		ErrorBufferInterface* errorhnd);

/// \brief Load a query analyzer program defined with a domain specific language
/// \param[in] analyzer analyzer to fill with the definitions in the source
/// \param[in] textproc textprocessor interface providing the builders for the objects defined
/// \param[in] filename the name of the file to parse in a domain specific language (see the syntax description for 'loadQueryAnalyzerProgramSource')
/// \param[in,out] errorhnd interface for reporting errors and exceptions occurred
bool loadQueryAnalyzerProgramFile(
		QueryAnalyzerInstanceInterface* analyzer,
		const TextProcessorInterface* textproc,
		const std::string& filename,
		ErrorBufferInterface* errorhnd);

}//namespace
#endif



