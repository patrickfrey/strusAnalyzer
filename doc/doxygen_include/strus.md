strusAnalyzer	 {#mainpage}
=============

The project strusAnalyzer provides some libraries for processing documents for feeding a search engine.
This process, also called document analysis extracts atomic units (terms and named patterns) for instertion into a strus storage for retrieval.

Document analysis process:
--------------------------
Document anaylsis is seen as a process involving the following three steps.

1. [Segmentation](@ref strus::SegmenterInterface)
	A document is splitted into segments of text. The segmentation is defined by expressions selecting the segments for further processing. The standard segmenter of strus uses a derivation of abbreviated syntax of XPath to select the segments.
	But you can define your own segmenter for any document format if it is possible to provide the segmenter interface for it.

2. [Tokenization](@ref strus::TokenizerFunctionInterface)
	A segment delivered by the document segmenter or a query phrase is split into tokens. The tokenization does not change the items selected. It just provides the start and end position in the original source.

3. [Pattern Matching](@ref strus::PatternMatcherInterface)
	A pattern matcher recognizes patterns of terms in the document and creates named units representing the matches that can be inserted into a storage as terms.
	We distinguish two forms of pattern matching:
	A. Pre Processing Pattern Matching: This type of pattern matching does it's own segmentation: [Pattern Lexer] (@ref strus::PatternLexerInterface)
	B. Post Processing Pattern Matching: This type of pattern matching processes terms created by the analyzer as basic lexems: [Pattern Term Feeder] (@ref strus::PatternTermFeederInterface)

4. [Normalization](@ref strus::NormalizerFunctionInterface)
	A token delivered by the tokenization is passed to a normalizer function. By normalizing terms you can impose rules of how to unify terms that should be mapped to the same value in the storage. Normalizer functions can be chained together to describe normalization in multiple steps where the input of a normalization function can be the output of a previous normalization step.

5. [Aggregation](@ref strus::AggregatorFunctionInterface)
	A document processed with all segmenters,tokenizers,normalizers defined can be passed to an aggregator function creating a numeric value. This numeric value can be used to represent some document statistics. Aggregator results are stored as meta data in the document.

Main interfaces and expandability:
----------------------------------
The main interfaces of the strusAnalyzer are
        [document analyzer interface](@ref strus::DocumentAnalyzerInterface)
and the [query analyzer interface](@ref strus::QueryAnalyzerInterface).
These two components can be instantiated with functions provided by the [text processor](@ref strus::TextProcessorInterface).




