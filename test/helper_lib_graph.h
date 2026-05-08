#pragma once
#include <stdlib.h>

#ifndef RDF_TERMTYPE_DEFINED
#define RDF_TERMTYPE_DEFINED
typedef enum {
        URI = 0,
        BNODE = 1,
        TYPEDLITERAL = 2,
        LANGLITERAL = 3
} TERMTYPE;
#endif


typedef struct tripleStream TripleStream;

TripleStream* new_TripleStream();
void free_TripleStream(TripleStream*);

/**
 * Append next triple. Dont reuse old and will generate new list if old is NULL.
 *
 * subject_type can only be HELPER_LIB_GRAPH_BNODE or HELPER_LIB_GRAPH_URI.
 *
 * Special behaviour depending on object_value:
 * 	for bnodes and uri the object_suffix, will be ignored.
 * 	for uri if object_suffix is NULL or "" it will be overriden with
 * 	xsd:string("http://www.w3.org/2001/XMLSchema#string")
 * 	for lang literal this will memory leak if object_suffix is NULL
 *
 * Memory leak for object_suffix = NULL
 * and object_type = HELPER_LIB_GRAPH_LANG_LITERAL
 */
void append_TripleStream(TripleStream* old,
		const char* subject_value, HELPER_LIB_GRAPH_NODETYPE subject_type,
		const char* predicate_value,
		const char* object_value, const char* object_suffix,
		HELPER_LIB_GRAPH_NODETYPE object_type,
		const char* graph, HELPER_LIB_GRAPH_NODETYPE graph_type);

/**
 * Check Isomorphims of the given rdf graphs.
 */
bool compare_triples(TripleStream* first, TripleStream* second);
