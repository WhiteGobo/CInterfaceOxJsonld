#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <jsonld.h>
#include <string.h>
#include <getopt.h>


const char* inputfile = NULL;

static void print_term(FILE* out, const char* x, const char* suffix, TERMTYPE type);
static int8_t my_triplehandler(
                const char* subject, uint8_t subject_type,
                const char* predicate,
                const char* object, const char* object_suffix,
                uint8_t object_type,
                const char* graphid, uint8_t graph_type,
                void* user);

static int parse_args(int argc, char *argv[]);
static char* load_input_into_memory();

int main(int argc, char* argv[]){
	int err;
	char* input;
	switch(parse_args(argc, argv)){
		case 0:
			break;
		default:
			exit(EXIT_FAILURE);
	}
	input = load_input_into_memory();
	if (input == NULL){
		fprintf(stderr, "Couldnt load inputfile \"%s\"\n", inputfile);
		exit(EXIT_FAILURE);
	}
	err = parse_jsonld(input, my_triplehandler, NULL);
	free(input);
			exit(EXIT_FAILURE);
	switch(err){
		case 0:
        		exit(EXIT_SUCCESS);
		default:
			exit(EXIT_FAILURE);
	}
}


static struct option parse_options[] = {
	{"input", required_argument, NULL, 'i'},
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
			case 'i':
				inputfile = optarg;
				break;
			default:
				fprintf(stderr, "unrecognized argument\n");
				err = 1;
				break;
		}
	}
	if (inputfile == NULL){
		fprintf(stderr, "Missing inputfile\n");
		err = 1;
	}
	return err;
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


static int8_t my_triplehandler(
                const char* subject, uint8_t subject_type,
                const char* predicate,
                const char* object, const char* object_suffix,
                uint8_t object_type,
                const char* graphid, uint8_t graph_type,
                void* user)
{
	if(graphid != NULL){
		fprintf(stderr, "quad(");
	} else {
		fprintf(stderr, "triple(");
	}
	print_term(stderr, subject, NULL, subject_type);
	fprintf(stderr, ", ");
	print_term(stderr, predicate, NULL, URI);
	fprintf(stderr, ", ");
	print_term(stderr, object, object_suffix, object_type);
	if(graphid != NULL){
		fprintf(stderr, ", ");
		print_term(stderr, graphid, NULL, URI);
	}
	fprintf(stderr, ")\n");
	if (subject == NULL || predicate == NULL || object == NULL){
		fprintf(stderr, "failed triplehandler\n");
		return 1;
	}
	/*append_TripleStream(qq,
			subject, subject_type,
			predicate,
			object, object_suffix, object_type,
			graphid, graph_type);
			*/
	return 0;
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
