#pragma once

#include "nquads_parser.h"

typedef struct term {
	char* value;
	char* suffix;
	NQUADS_TERMTYPE type;
} Term;
