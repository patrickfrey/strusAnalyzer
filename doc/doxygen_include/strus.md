strusAnalyzer	 {#mainpage}
=============

The project strusAnalyzer provides some libraries for processing documents for feeding a search engine.
This process, also called document document analysis splits a document into normalized atomic terms that can be insterted into a strus storage for retrieval.
As counterpart strusAnalyzer provides also the methods to normalize query terms and phrases for the search in a strus storage.


Document analysis process:
--------------------------
Document anaylsis is seen as a process involving the following steps.

1. [Document segmentation](@ref strus::SegmenterInterface)
	A document is splitted into segments of text. The segmentation is defined by expressions selecting the segments for further processing. The standard segmenter of strus (libstrus_segmenter_textwolf) uses a derivation of abbreviated syntax of XPath to select the segments.

2. [Segment tokenization](@ref strus::TokenizerInterface)
	A segment delivered by the segmenter is split into tokens. The tokenization does not change the items selected. It just marks them by start and end position in the original source.

3. [Token normalization](@ref strus::NormalizerInterface)
	A token delivered by the tokenization is passed to a normalizer function. By normalizing terms you can impose rules of how to unify terms that should be mapped to the same value in the storage. Normalizer functions can be chained together to define normalization as composition of normalizers.


Main interfaces and expandability:
----------------------------------
The main interfaces of the strusAnalyzer are
        [document analyzer interface](@ref strus::DocumentAnalyzerInterface)
and the [query analyzer interface](@ref strus::QueryAnalyzerInterface).
These two components use functions provided by the [text processor](@ref strus::TextProcessorInterface).
You can define new functions and add them to the text processor and you can substitute
existing functions by alternative implementations.
The document segmenter is passed the the document analyzer when creating it.
You can define your own for any document format if it is possible to implement the segmenter interface for it.
To define the components of your analyzer dynamically and driven by runtime settings, 
have a look at the project <a href="https://github.com/patrickfrey/strusModule">strusModule</a>.


List of libraries:
------------------
* libstrus_analyzer			Analyzer
* libstrus_textproc			Container for text processing functions.
* libstrus_segmenter_textwolf		XML document segmenter (textwolf)
* libstrus_normalizer_snowball		Normalizer for stemming (snowball)
* libstrus_normalizer_dictmap		Mapping of values with help of a map
* libstrus_normalizer_charconv		Character conversions like for example to lower case (lc).
* libstrus_tokenizer_word		Tokenizer to split a text into words
* libstrus_tokenizer_punctuation	Tokenizer for end of sentence markers (for german only, using some silly heuristics that are not verified)



