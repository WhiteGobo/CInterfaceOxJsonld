#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "CInterfaceOxJsonld.h"
#include <fcntl.h>
//#include "helper_lib_graph.h"
//#include "rusttools.h"
#include "BasicRDFGraphComparator.h"
#include <string.h>

static char* myserialize(const char* input);
static bool compare_two_graphs(const char* x, const char* y);

const char* input = "\
{\
  \"@id\": \"http://greggkellogg.net/foaf#me\",\
  \"http://xmlns.com/foaf/0.1/name\": \"Gregg Kellogg\"\
}\
";

int main(int argc, char *argv[]){
	bool success;
	int err;
	char* serialized;
	serialized = myserialize(input);
	if (serialized == NULL){
		fprintf(stderr, "Serialize failed\n");
		exit(EXIT_FAILURE);
	}
	success = compare_two_graphs(input, serialized);
	free(serialized);
	if (success){
		err = EXIT_SUCCESS;
	} else {
		err = EXIT_FAILURE;
	}
	exit(err);
}

static char* myserialize(const char* input){
	int err;
	JSONLDSerializer* serializer_cfg = JSONLD_SER_start();
	err = parse_jsonld(input, (TripleHandler*) JSONLD_SER_add,
                                serializer_cfg, NULL);
	if (err != 0){
		fprintf(stderr, "Failed to parsing.");
		return NULL;
	}
	return JSONLD_SER_finish(serializer_cfg);
}


static bool compare_two_graphs(const char* x, const char* y){
	bool are_same_graphs;
	TripleStream *initial_triples = new_TripleStream();
        TripleStream *last_triples = new_TripleStream();

	int err;
	err = parse_jsonld(x, (TripleHandler*) append_TripleStream, initial_triples, NULL);
	if (err != 0){
		fprintf(stderr, "Failed to reparse input:\n%s\n", x);
		return false;
	}
	err = parse_jsonld(y, (TripleHandler*) append_TripleStream, last_triples, NULL);
	if (err != 0){
		fprintf(stderr, "Failed to parse newly serialized:\n%s\n", y);
        	free_TripleStream(initial_triples);
		return false;
	}
        are_same_graphs = compare_triples(initial_triples, last_triples);
        free_TripleStream(initial_triples);
        free_TripleStream(last_triples);
        if (are_same_graphs){
		return true;
	}
	fprintf(stderr, "expected serialized and parsed to be the "
			"same info\nas input:\n%s\n\nserialized to:\n%s\n", x, y);
	return false;
}
