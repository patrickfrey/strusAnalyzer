cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory( randomfeed )
add_subdirectory( documentclassdetect )
add_subdirectory( segmenter_textwolf )
add_subdirectory( segmenter_cjson )
add_subdirectory( segmenter_tsv )
add_subdirectory( tokenizer_textcat )
add_subdirectory( tokenizer_regex )
add_subdirectory( posbind )
add_subdirectory( normalizer_regex )
add_subdirectory( querytree )
# add_subdirectory( prgload )

