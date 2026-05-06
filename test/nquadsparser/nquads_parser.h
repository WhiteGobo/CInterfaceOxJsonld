#pragma once

#include <stdint.h>

typedef enum {
	NQUADS_URI = 0,
	NQUADS_BNODE = 1,
	NQUADS_TYPEDLITERAL = 2,
	NQUADS_LANGLITERAL = 3
} NQUADS_TERMTYPE;

typedef int8_t NQUADS_TripleHandler(
                const char* subject, uint8_t subject_type,
                const char* predicate,
                const char* object, const char* object_suffix,
                uint8_t object_type,
                const char* graphid, uint8_t graph_type,
                void* user);

/**
 * Parse given nquads file and call triplehandler with any found triple.
 * Returns 0 on success.
 */
int nquads_parse_file(const char* filename, NQUADS_TripleHandler* triplehandler, void* user);
