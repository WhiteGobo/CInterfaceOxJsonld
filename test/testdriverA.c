#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "CInterfaceOxJsonld.h"
#include <fcntl.h>
//#include "helper_lib_graph.h"
//#include "rusttools.h"
#include "BasicRDFGraphComparator.h"
#include "nquads_parser.h"
#include <string.h>
#include "cwalk.h"

static int parse_args(int argc, char *argv[]);
static int check_negative_test_result(int err);

static int8_t my_triplehandler(
                const char* subject, uint8_t subject_type,
                const char* predicate,
                const char* object, const char* object_suffix,
                uint8_t object_type,
                const char* graph_id, uint8_t graph_type,
                void* user);

static char* load_input_into_memory();


typedef enum {
	ID_AUTO = 0,
	ID_v10 = 1,
	ID_v11 = 2
} VERSION_ID;

typedef enum {
	PositiveEvaluationTest,
	NegativeEvaluationTest,
	PositiveSyntaxTest,
	NegativeSyntaxTest,
} TESTTYPE;

TESTTYPE testtype = PositiveEvaluationTest;
VERSION_ID proc_mode = ID_AUTO;
VERSION_ID spec_version = ID_AUTO;
const char* inputfile = NULL;
const char* expectfile = NULL;
const char* base_uri = NULL;
const char* testname = NULL;
const char* purpose = NULL;
bool load_over_http = false;
//bool negative_test = false;
TripleStream* input_graph;
TripleStream* test_graph;

int main(int argc, char *argv[]){
	int err;
	int fd;
	bool are_same_graphs;
	err = parse_args(argc, argv);
	if (testname != NULL){
		fprintf(stderr, "testname:: %s\n", testname);
	}
	if (purpose != NULL){
		fprintf(stderr, "purpose: %s\n", purpose);
	}
	switch(err){
		case 0:
			break;
		case 2:
			fprintf(stderr, "missing inputfile.\n");
			exit(EXIT_FAILURE);
		case 3:
			fprintf(stderr, "missing expectfile "
					"and not negative syntax test.\n");
			exit(EXIT_FAILURE);
		default:
			fprintf(stderr, "unhandled broken script input.\n");
			exit(EXIT_FAILURE);
	}
	input_graph = new_TripleStream();
	fprintf(stderr, "parse inputfile: %s\n", inputfile);
	const char *input = load_input_into_memory();
	if (input == NULL){
		fprintf(stderr, "Missing input file: %s\n", inputfile);
		exit(EXIT_FAILURE);
	}
	size_t end_dirname;
	cwk_path_get_dirname(inputfile, &end_dirname);
	char* dirname = malloc(end_dirname + 1);
	memcpy(dirname, inputfile, end_dirname);
	dirname[end_dirname] = '\0';
	const char* q = strrchr(base_uri, '/');
	size_t end_base_uri = q - base_uri;
	char *base_relative_iris = malloc(end_base_uri + 1);
	memcpy(base_relative_iris, base_uri, end_base_uri);
	base_relative_iris[end_base_uri] = '\0';
	JSONLDConfig* config = JSONLDConfig_enable_LoadDocumentCallback_for_relativefiles(NULL, base_relative_iris, dirname);
	free(base_relative_iris);
	free(dirname);

	if (config == NULL){
		fprintf(stderr, "Emergency break.\n");
		exit(EXIT_FAILURE);
	}
	//config = JSONLDConfig_enable_LoadDocumentCallback_for_localfiles(config);
	switch(proc_mode){
		case ID_v10:
			config = JSONLDConfig_use_processing_mode_10(config);
			break;
		case ID_v11:
			config = JSONLDConfig_use_processing_mode_11(config);
	}
	config = JSONLDConfig_set_baseiri(config, base_uri);
	if (config==NULL){
		fprintf(stderr, "Failed to read base iri.");
		exit(EXIT_FAILURE);
	}
	if (load_over_http){
		config = JSONLDConfig_enable_LoadDocumentCallback_over_http(config);
		if (config==NULL){
			fprintf(stderr, "Failed to enable loading over http.");
			exit(EXIT_FAILURE);
		}
	}
	err = parse_jsonld(input, my_triplehandler, input_graph, config);
	free(input);
	switch(testtype) {
		case NegativeSyntaxTest:
			free_JSONLDConfig(config);
			free_TripleStream(input_graph);
			switch(err){
				case 0:
					fprintf(stderr, "Succeeded but expected "
							"failure\n");
					exit(EXIT_FAILURE);
				default:
					exit(EXIT_SUCCESS);
			}
			break;
	}
	switch(err){
		case 0:
			break;
		default:
			free_JSONLDConfig(config);
			free_TripleStream(input_graph);
			fprintf(stderr, "parsing of inputfile failed\n");
			exit(EXIT_FAILURE);
	}
	if (expectfile == NULL){
		free_JSONLDConfig(config);
		free_TripleStream(input_graph);
		exit(EXIT_SUCCESS);
	}
	fprintf(stderr, "\nparse expectfile: %s\n", expectfile);
	test_graph = new_TripleStream();
	err = nquads_parse_file(expectfile,
			(NQUADS_TripleHandler*) my_triplehandler, test_graph);
	switch(err){
		case 0:
			break;
		default:
			free_JSONLDConfig(config);
			free_TripleStream(input_graph);
			free_TripleStream(test_graph);
			fprintf(stderr, "parsing of expectfile failed\n");
			exit(EXIT_FAILURE);
	}

	fprintf(stderr, "\ncompare triples\n");
	are_same_graphs = compare_triples(input_graph, test_graph);
	if (are_same_graphs){
		fprintf(stderr, "input and expect are isomorph\n");
	} else {
		fprintf(stderr, "input and expect are not isomorph\n");
	}
	free_JSONLDConfig(config);
	free_TripleStream(input_graph);
	free_TripleStream(test_graph);
	switch(testtype) {
		case PositiveEvaluationTest:
			if (!are_same_graphs){
				fprintf(stderr, "JSONLD input graph isnt the "
						"same as test graph.\n");
				exit(EXIT_FAILURE);
			}
			break;
		case NegativeEvaluationTest:
			if (are_same_graphs){
				fprintf(stderr, "JSONLD input graph is the "
						"same as test graph.\n");
				exit(EXIT_FAILURE);
			}
			break;
	}
	exit(EXIT_SUCCESS);
}


static struct option parse_options[] = {
	{"PositiveEvaluationTest", no_argument, NULL, '+'},
	{"PositiveSyntaxTest", no_argument, NULL, 'S'},
	{"NegativeEvaluationTest", no_argument, NULL, '-'},
	{"NegativeSyntaxTest", no_argument, NULL, 's'},
	{"name", required_argument, NULL, 'n'},
	{"purpose", required_argument, NULL, 'p'},
	{"expect", required_argument, NULL, 'e'},
	{"input", required_argument, NULL, 'i'},
	{"base-uri", required_argument, NULL, 'b'},
	{"version", required_argument, NULL, 'v'},
	{"processing-mode", required_argument, NULL, 'm'},
	{"enable-load-over-http", no_argument, NULL, 'I'},
        {NULL, 0, NULL, 0}
};


static int parse_args(int argc, char *argv[]){
	int err = 0;
	int c = 0;
	int option_index;
	while(c != -1){
		c = getopt_long(argc, argv, "",
				parse_options, &option_index);
		switch(c){
			case -1: //end of arguments
				break;
			case 'n':
				testname = optarg;
				break;
			case 'v':
				if(0 == strcmp(optarg, "1.0")){
					spec_version = ID_v10;
				} else if (0 == strcmp(optarg, "1.1")){
					spec_version = ID_v11;
				}
				break;
			case 'm':
				if(0 == strcmp(optarg, "1.0")){
					proc_mode = ID_v10;
				} else if (0 == strcmp(optarg, "1.1")){
					proc_mode = ID_v11;
				}
				break;
			case 'I':
				load_over_http = true;
				break;
			case 'p':
				purpose = optarg;
				break;
			case 'i':
				fprintf(stderr, "inputfile: %s\n", optarg);
				inputfile = optarg;
				break;
			case 'e':
				fprintf(stderr, "expectfile: %s\n", optarg);
				expectfile = optarg;
				break;
			case 'b':
				fprintf(stderr, "baseuri: %s\n", optarg);
				base_uri = optarg;
				break;
			case '+':
				testtype = PositiveEvaluationTest;
				break;
			case '-':
				testtype = NegativeEvaluationTest;
				break;
			case 's':
				testtype = NegativeSyntaxTest;
				break;
			default:
				fprintf(stderr, "unrecognized argument\n");
				err = 1;
				break;
		}
	}
	if (inputfile == NULL){
		return 2;
	}
	return err;
}

static void print_term(FILE* out, const char* x, const char* suffix, TERMTYPE type){
	switch (type){
		case URI:
			fprintf(out, "<%s>", x);
			break;
		case BNODE:
			fprintf(out, "_:%s", x);
			break;
		case TYPEDLITERAL:
			fprintf(out, "\"%s\"", x);
			if (suffix != NULL && 0 != strcmp(suffix, "")){
				fprintf(out, "^^%s", suffix);
			}
			break;
		case LANGLITERAL:
			fprintf(out, "\"%s\"@%s", x, suffix);
			break;
	}
}

static int8_t my_triplehandler(
                const char* subject, uint8_t subject_type,
                const char* predicate,
                const char* object, const char* object_suffix,
                uint8_t object_type,
                const char* graph_id, uint8_t graph_type,
                void* user)
{
	if(graph_id != NULL){
		fprintf(stderr, "quad(");
	} else {
		fprintf(stderr, "triple(");
	}
	TripleStream* qq = (TripleStream*) user;
	print_term(stderr, subject, NULL, subject_type);
	fprintf(stderr, ", ");
	print_term(stderr, predicate, NULL, URI);
	fprintf(stderr, ", ");
	print_term(stderr, object, object_suffix, object_type);
	if(graph_id != NULL){
		fprintf(stderr, ", ");
		print_term(stderr, graph_id, NULL, URI);
	}
	fprintf(stderr, ")\n");
	if (subject == NULL || predicate == NULL || object == NULL){
		fprintf(stderr, "failed triplehandler\n");
		return 1;
	}
	append_TripleStream(
			subject, subject_type,
			predicate,
			object, object_suffix, object_type,
			graph_id, graph_type,
			qq);
	return 0;
}

static int check_negative_test_result(int err){
	switch(err){
		case 0:
			fprintf(stderr, "negative test didnt "
					"fail on parsing.\n");
			return EXIT_FAILURE;
		case 1234:
			return EXIT_SUCCESS;
		default:
			fprintf(stderr, "negative test didnt fail on parsing "
					"with handled errorcode %d.\n", err);
			return EXIT_FAILURE;
	}
}

static char* load_input_into_memory(){
	char *ret;
	long fsize;
	FILE *f = fopen(inputfile, "rb");
	if (f == NULL) return NULL;
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	rewind(f);
	//fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

	ret = malloc(fsize + 1);
	fread(ret, fsize, 1, f);
	ret[fsize] = 0;
	fclose(f);
	return ret;
}
