cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(src)

add_test( DocumentClassDetectXMLUTF8 src/testDocumentClassDetect "${PROJECT_SOURCE_DIR}/tests/documentclassdetect" application/xml UTF-8 xml-utf8.xml )
add_test( DocumentClassDetectXMLUTF8BOM src/testDocumentClassDetect "${PROJECT_SOURCE_DIR}/tests/documentclassdetect" application/xml UTF-8 xml-utf8-BOM.xml )

add_test( DocumentClassDetectJSONUTF8 src/testDocumentClassDetect "${PROJECT_SOURCE_DIR}/tests/documentclassdetect" application/json UTF-8 json-utf8.json )

add_test( DocumentClassDetectTSVUTF8 src/testDocumentClassDetect "${PROJECT_SOURCE_DIR}/tests/documentclassdetect" text/tab-separated-values UTF-8 tsv-utf8.tsv )
