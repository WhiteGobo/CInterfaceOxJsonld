/**
 *
 * No further resources to point to, to explain the algorithm, sorry:(
 *
 * Uses [hashtable](https://github.com/xtreme8000/hashtable),
 * which is licensed under [MIT License](https://goldsborough.mit-license.org/).
 */

#include "helper_lib_graph.h"
#include <stdlib.h>
#include <string.h>
//#include "hashtable.h"
#include <stdio.h>
#include <stdint.h>


#define INITIAL_HASHTABLE_SIZE 30
#define XSD_string "http://www.w3.org/2001/XMLSchema#string"


typedef enum {
	ID_FIRST_GRAPH,
	ID_SECOND_GRAPH
} GRAPH_ID;

typedef bool (*ht_iterate_t) (void* key, void* value, void* user);

typedef struct singleTriple {
	char* subject_value;
	TERMTYPE subject_type;
	char* predicate_value;
	char* object_value;
	TERMTYPE object_type;
	char* object_suffix;
} SingleTriple;

typedef HashTable TripleSet;
typedef struct key_TripleSet {
	SingleTriple* val;
} Key_TripleSet;
static size_t hash_TripleSet(Key_TripleSet* key, size_t keysize);
static int compare_TripleSet(Key_TripleSet* first_key, Key_TripleSet* second_key, size_t keysize);
static void add_TripleSet(TripleSet* triple_set, SingleTriple* key);
static int remove_TripleSet(TripleSet* triple_set, SingleTriple* key);


static void fprintf_term(FILE* stream, const char* value, TERMTYPE type, const char* suffix);
static void fprintf_triple(FILE* stream, SingleTriple* triple);

typedef struct tripleList {
	SingleTriple first;
	struct tripleList* rest;
} TripleList;

typedef struct tripleStream {
	TripleList* triples;
	TripleList* last;
} TripleStream;

typedef struct bnodeDescription BNodeDescription;

typedef struct bnodeDescriptionList {
	struct bnodeDescription *first;
	struct bnodeDescriptionList *next;
} BNodeDescriptionList;

typedef struct colorPalette ColorPalette;

/**
 * Vertice color needed to determine graph isomorphism
 */
typedef struct myColor {
	int index;
	//updated during refinement
	struct bnodeDescriptionList* bnode_list;
	struct myColor *next;
	//struct myColor *nontrivial_next;
	ColorPalette* parent;
} MyColor;

typedef struct colorPalette {
	MyColor* first;
	//MyColor* first_nontrivial;
	MyColor* last;
} ColorPalette;

/**
 * maps edges of a bnode to a new subcolor.
 *
 * Color is determined by starting color, and the in and out edges, defined by 
 * their respective predicate and the color of the other bnode(object or subject).
 */
typedef HashTable BNodeNeighbours2Color;
typedef struct key_BNodeNeighbours2Color {
	BNodeDescription* val;
} Key_BNodeNeighbours2Color;
typedef struct value_BNodeNeighbours2Color {
	MyColor* val;
} Value_BNodeNeighbours2Color;
static BNodeNeighbours2Color* new_BNodeNeighbours2Color();
static void free_BNodeNeighbours2Color(BNodeNeighbours2Color* x);
static int compare_BNodeNeighbours2Color(Key_BNodeNeighbours2Color *first, Key_BNodeNeighbours2Color *second, size_t keysize);
static size_t hash_BNodeNeighbours2Color(Key_BNodeNeighbours2Color *key, size_t keysize);
static int insert_BNodeNeighbours2Color(BNodeNeighbours2Color* table, MyColor* color);
static MyColor* get_BNodeNeighbours2Color(BNodeNeighbours2Color* table, BNodeDescription* bnode);

typedef struct bnodeEdge {
	const char* property_value;
	struct bnodeDescription* target;
} BNodeEdge;

typedef struct edgePartList {
	BNodeEdge* first;
	struct edgePartList* next;
} EdgePartList;

/**
 * This contains information, how colors are refined for each node.
 */
typedef struct bnodeDescription {
	const char* bnode_id;
	GRAPH_ID graph_id;
	//updated after each refinement
	struct myColor *last_color;
	BNodeEdge** in;
	size_t in_length;
	BNodeEdge** out;
	size_t out_length;
} BNodeDescription;

static ColorPalette* create_initial_color_and_check_trivial_edges(
		TripleList* first, TripleList* second);
static bool check_color_balance(ColorPalette* colors);
//static void filter_trivial_colors(ColorPalette* start_colors);
static bool refine(ColorPalette* colors);
static int number_colors(ColorPalette* colors);
static void write_coloring(ColorPalette* colors);
static void free_bnodes(BNodeDescriptionList* all_bnodes);
static BNodeDescriptionList* insert_BNodeDescriptionList(BNodeDescriptionList*, BNodeDescription*);
static MyColor* get_last_color(MyColor*);
static int compare_color(BNodeDescription *first, BNodeDescription *second);
//static void generate_or_assign_color(MyColor* available_colors, BNodeDescription *);
static EdgePartList* insert_EdgePartList(EdgePartList *start, const char* predicate, BNodeDescription *target);

static ColorPalette* new_ColorPalette();
static void free_ColorPalette_and_BNodes(ColorPalette*);
static MyColor* new_Color(ColorPalette* color_palette, BNodeDescription* bnode);
static void add_bnode_Color(MyColor* color, BNodeDescription* bnode);


static int compare_EdgePartList(EdgePartList* first, EdgePartList* second);
static void sort_edges_in_all_nodes_in_colors(
					ColorPalette* color_palette);

static bool equal_and_free_TripleSet(TripleSet* first, TripleSet* second);

static size_t extend_hash_str(size_t hash, const char* key);
static size_t extend_hash_int(size_t hash, int key);

typedef int (*qsort_call)(const void* , const void*);
static int compare_BNodeEdge(const BNodeEdge* first, const BNodeEdge* second);

void free_TripleStream(TripleStream* graph){
	TripleList *tmp1, *tmp2;
	tmp1 = graph->triples;
	while (tmp1 != NULL){
		tmp2 = tmp1;
		tmp1 = tmp1->rest;
		free(tmp2->first.subject_value);
		free(tmp2->first.predicate_value);
		free(tmp2->first.object_value);
		free(tmp2->first.object_suffix);
		free(tmp2);
	}
	free(graph);
}

TripleStream* new_TripleStream(){
	TripleStream* new = malloc(sizeof(TripleStream));
	new->triples = NULL;
	new->last = NULL;
	return new;
}

void append_TripleStream(TripleStream* stream,
		const char* subject_value, TERMTYPE subject_type,
		const char* predicate_value,
		const char* object_value, const char* object_suffix,
		TERMTYPE object_type,
		const char* graph, TERMTYPE graph_type)
{
	TripleList *new;
	new = malloc(sizeof(TripleList));
	new->rest = NULL;
	new->first.subject_value = malloc(strlen(subject_value) + 1);
	strcpy(new->first.subject_value, subject_value);
	new->first.subject_type = subject_type;
	new->first.predicate_value = malloc(strlen(predicate_value) + 1);
	strcpy(new->first.predicate_value, predicate_value);
	new->first.object_value = malloc(strlen(object_value) + 1);
	strcpy(new->first.object_value, object_value);
	switch (object_type){
		case HELPER_LIB_GRAPH_URI:
		case HELPER_LIB_GRAPH_BNODE:
			new->first.object_suffix = NULL;
			break;
		case HELPER_LIB_GRAPH_TYPED_LITERAL:
			if (object_suffix == NULL
					|| 0 == strcmp("", object_suffix))
			{
				object_suffix = XSD_string;
			}
		case HELPER_LIB_GRAPH_LANG_LITERAL:
			new->first.object_suffix = malloc(strlen(object_suffix) + 1);
			strcpy(new->first.object_suffix, object_suffix);
			break;
	}
	new->first.object_type = object_type;
	if (stream->last != NULL){
		stream->last->rest = new;
		stream->last = new;
	} else {
		stream->triples = new;
		stream->last = new;
	}
	SingleTriple* triple = &new->first;
}


bool compare_triples(TripleStream* first, TripleStream* second){
	bool done_refinement = true;
	bool success = true;
	size_t i, max_i;
	ColorPalette* colors;
	colors = create_initial_color_and_check_trivial_edges(first->triples, second->triples);
	if (colors == NULL){
		return false;
	}
	if (!check_color_balance(colors)){
		fprintf(stderr, "bnodes are different in both graphs\n");
		success = false;
	} else {
		i = 0;
		max_i = number_colors(colors);
		while(done_refinement && success && i<max_i){
			i++;
			//filter_trivial_colors(colors);
			sort_edges_in_all_nodes_in_colors(colors);
			done_refinement = refine(colors);
			if (!check_color_balance(colors)){
				fprintf(stderr, "bnodes are different "
						"in both graphs\n");
				success = false;
				break;
			}
			write_coloring(colors);
		}
	}
	free_ColorPalette_and_BNodes(colors);
	return success;
}

static bool check_color_balance(ColorPalette* colors){
	size_t number_first=0, number_second=0;
	for(MyColor* x = colors->first;
			x != NULL; x = x->next){
		number_first=0;
		number_second=0;
		for(BNodeDescriptionList* tmplist = x->bnode_list;
				tmplist != NULL;
				tmplist = tmplist->next)
		{
			if (tmplist->first->graph_id == ID_FIRST_GRAPH){
				number_first++;
			} else {
				number_second++;
			}
		}
		if (number_first != number_second){
			return false;
		}
	}
	return true;
}

static void free_ColorPalette_and_BNodes(ColorPalette* colors){
	MyColor *current, *next;
	current = colors->first;
	while (current != NULL){
		next = current->next;
		free_bnodes(current->bnode_list);
		free(current);
		current = next;
	}
	free(colors);
}

static void write_coloring(ColorPalette* colors){
	for (MyColor *color = colors->first;
			color != NULL;
			color = color->next)
	{
		for (BNodeDescriptionList *bnodelist = color->bnode_list;
				bnodelist!= NULL;
				bnodelist=bnodelist->next)
		{
			bnodelist->first->last_color = color;
		}

	}
}

static int count_length_BNodeDescriptionList(BNodeDescriptionList *bnodelist){
	int number_bnodes = 0;
	for (BNodeDescriptionList *bn=bnodelist; bn!=NULL; bn = bn->next)
	{
		number_bnodes++;
	}
	return number_bnodes;
}

static void sort_edges_in_all_nodes_in_colors(ColorPalette* color_palette)
{
	BNodeDescription* x;
	for (MyColor* color = color_palette->first;
			color != NULL;
			color = color->next)
	{
		for (BNodeDescriptionList* bnodes = color->bnode_list;
				bnodes != NULL;
				bnodes = bnodes->next)
		{
			x = bnodes->first;
			if (x->in == NULL){
				fprintf(stderr, "no in; ");
			}
			if (x->out == NULL){
				fprintf(stderr, "no out; ");
			}
			fprintf(stderr, "inl: %d, outl: %d\n", x->in_length, x->out_length);
			qsort(x->in, x->in_length, sizeof(BNodeEdge),
					(qsort_call) compare_BNodeEdge);
			qsort(x->out, x->out_length, sizeof(BNodeEdge),
					(qsort_call) compare_BNodeEdge);
		}
	}
}

/**
 * The algorithm expects, that the colors are balanced existing in both graphs.
 * This means, that trivial colors contains exactly 2 bnodes. This will be used
 * within the algorithm.
 */
/*
static void filter_trivial_colors(ColorPalette* start_colors){
	int number_bnodes = -1;
	MyColor *last;
	MyColor *first = start_colors->first_nontrivial;
	if (first == NULL) return;
	for (MyColor *x = first;
			x != NULL && number_bnodes != 2;
			x = x->nontrivial_next)
	{
		first = x;
		number_bnodes = count_length_BNodeDescriptionList(x->bnode_list);
	}
	last = first;
	for (MyColor *x = first->nontrivial_next; x!=NULL; x=x->nontrivial_next)
	{
		number_bnodes = count_length_BNodeDescriptionList(x->bnode_list);
		if (number_bnodes != 2){
			last->nontrivial_next = x;
			last = x;
		}
	}
	last->nontrivial_next = NULL;
	start_colors->first_nontrivial = first;
}
*/

static void free_bnodes(BNodeDescriptionList* all_bnodes){
	BNodeDescriptionList *current = all_bnodes, *next=NULL;
	while (current != NULL){
		next = current->next;
		for (int i = 0; i<current->first->in_length; i++){
			free(current->first->in[i]);
		}
		free(current->first->in);
		for (int i = 0; i<current->first->out_length; i++){
			free(current->first->out[i]);
		}
		free(current->first->out);
		free(current->first);
		free(current);
		current = next;
	}
}

static BNodeDescriptionList* insert_BNodeDescriptionList(
		BNodeDescriptionList* start, BNodeDescription* first)
{
	BNodeDescriptionList* new = malloc(sizeof(BNodeDescriptionList));
	new->first = first;
	if (start != NULL){
		new->next = start->next;
		start->next = new;
		return start;
	} else {
		new->next = NULL;
		return new;
	}
}

static void remove_second_BNodeDescriptionList(BNodeDescriptionList *last){
	BNodeDescriptionList* tmp = last->next;
	if (tmp != NULL){
		last->next = tmp->next;
		free(tmp);
	}
}

/**
 * returns true if something was refined.
 */
static bool refine(ColorPalette* colors){
	bool done_something = false;
	MyColor *break_at_this_color = colors->last;
	BNodeDescription *current_bnode;
	MyColor* current_color, *new_color;
	BNodeNeighbours2Color *sub_colors;
	//points to last MyColor. updated each refinement step
	//Used in generate_or_assign_color
	BNodeDescriptionList *last, *current, *next;
	for(MyColor* x = colors->first;
			x != NULL;
			x = x->next)
	{
		current_color = x;
		sub_colors = new_BNodeNeighbours2Color();
		insert_BNodeNeighbours2Color(sub_colors, current_color);
		last = x->bnode_list;
		current = last->next;//skip first
		while (current != NULL){
			next = current->next;
			current_bnode = current->first;
			new_color = get_BNodeNeighbours2Color(sub_colors, current_bnode);
			if (new_color == NULL){
				new_color = new_Color(x->parent, current_bnode);
				insert_BNodeNeighbours2Color(sub_colors, new_color);
				remove_second_BNodeDescriptionList(last);
				done_something = true;
			} else {
				last = current;
			}
			current = next;
		}
		free_BNodeNeighbours2Color(sub_colors);
		if (x == break_at_this_color) break;
	}
	return done_something;
}

static MyColor* get_BNodeNeighbours2Color(BNodeNeighbours2Color* table, BNodeDescription* bnode)
{
	Key_BNodeNeighbours2Color key = {.val = bnode};
	Value_BNodeNeighbours2Color* value;
	value = ht_lookup(table, &key);
	if (value != NULL){
		return value->val;
	} else {
		return NULL;
	}
}

static int insert_BNodeNeighbours2Color(BNodeNeighbours2Color* table, MyColor* color)
{
	int err;
	Key_BNodeNeighbours2Color key = {.val = color->bnode_list->first};
	Value_BNodeNeighbours2Color value = {.val = color};
	ht_insert(table, &key, &value);
	return err;
}

static BNodeNeighbours2Color* new_BNodeNeighbours2Color(){
	BNodeNeighbours2Color* new = malloc(sizeof(BNodeNeighbours2Color));
	ht_setup(new, sizeof(Key_BNodeNeighbours2Color), sizeof(Value_BNodeNeighbours2Color), INITIAL_HASHTABLE_SIZE);
	new->hash = (hash_t) hash_BNodeNeighbours2Color;
	new->compare = (comparison_t) compare_BNodeNeighbours2Color;
	return new;
}

static void free_BNodeNeighbours2Color(BNodeNeighbours2Color* x){
	ht_destroy(x);
	free(x);
}


static int compare_BNodeNeighbours2Color(Key_BNodeNeighbours2Color *first, Key_BNodeNeighbours2Color *second, size_t keysize){
	int ret;
	EdgePartList *edge_first, *edge_second;
	BNodeDescription *x = first->val, *y = second->val;
	ret = compare_color(x, y);
	return ret;
}

static size_t extend_hash_BNodeEdge(size_t hash, BNodeEdge *x){
	uintptr_t key_value = (uintptr_t) x->target->last_color;
	//hash = ((hash << 5) + hash) + key_value;
	hash = extend_hash_str(hash, x->property_value);
	return hash;
}

static size_t hash_BNodeNeighbours2Color(Key_BNodeNeighbours2Color *key_ptr, size_t keysize)
{
	size_t hash = 5381;
	EdgePartList *edge_first, *edge_second;
	BNodeDescription *key = key_ptr->val;
	for (int i = 0; i < key->in_length; i++){
		hash = extend_hash_BNodeEdge(hash, key->in[i]);
	}
	for (int i = 0; i < key->out_length; i++){
		hash = extend_hash_BNodeEdge(hash, key->out[i]);
	}
	return hash;
}


static size_t extend_hash_int(size_t hash, int key){
	hash = ((hash << 5) + hash) + key;
}

static size_t extend_hash_str(size_t hash, const char* key){
	for(int i=0; key[i] != '\0'; i++){
		hash = ((hash << 5) + hash) + key[i];
	}
	return hash;
}
static size_t hash_TripleSet(Key_TripleSet* key_ptr, size_t keysize){
	SingleTriple* key = key_ptr->val;
	size_t hash = 5381;
	if (key->subject_type != HELPER_LIB_GRAPH_BNODE){
		hash = extend_hash_str(hash, key->subject_value);
	}
	hash = ((hash << 5) + hash) + key->subject_type;
	hash = extend_hash_str(hash, key->predicate_value);
	if (key->object_type != HELPER_LIB_GRAPH_BNODE){
		hash = extend_hash_str(hash, key->object_value);
	}
	if (key->object_suffix == NULL){
		hash = extend_hash_str(hash, "");
		//hash = ((hash << 5) + hash) + 0;
	} else {
		hash = extend_hash_str(hash, key->object_suffix);
	}
	hash = ((hash << 5) + hash) + key->object_type;
	return hash;
}

static int compare_TripleSet(Key_TripleSet* first_key, Key_TripleSet* second_key, size_t keysize){
	int ret;
	SingleTriple* first = first_key->val;
	SingleTriple* second = second_key->val;

	ret = first->subject_type - second->subject_type;
	if (ret != 0) return ret;
	ret = first->object_type - second->object_type;
	if (ret != 0) return ret;
	if (first->subject_type != HELPER_LIB_GRAPH_BNODE){
		ret = strcmp(first->subject_value, second->subject_value);
		if (ret != 0) return ret;
	}
	if (first->object_type != HELPER_LIB_GRAPH_BNODE){
		ret = strcmp(first->object_value, second->object_value);
		if (ret != 0) return ret;
	}
	if (first->object_suffix == NULL){
		if (second->object_suffix != NULL) return 1;
	} else if (second->object_suffix == NULL){
		return -1;
	} else {
		ret = strcmp(first->object_suffix, second->object_suffix);
		if (ret != 0) return ret;
	}
	ret = strcmp(first->predicate_value, second->predicate_value);
	return ret;
}

static TripleSet* setup_TripleSet(){
	TripleSet* ret = malloc(sizeof(TripleSet));
	ht_setup(ret, sizeof(Key_TripleSet), sizeof(Key_TripleSet), INITIAL_HASHTABLE_SIZE);
	ret->hash = (hash_t) hash_TripleSet;
	ret->compare = (comparison_t) compare_TripleSet;
	return ret;
}

static void destroy_TripleSet(TripleSet* set){
	ht_destroy(set);
	free(set);
}


/**
 * Add trivial triple(no bnode) to tiple_set, needed for trivial graph equality.
 */
static void add_TripleSet(TripleSet* triple_set, SingleTriple* key)
{
	Key_TripleSet insert_key = {.val=key};
	ht_insert(triple_set, &insert_key, &insert_key);
}

static int remove_TripleSet(TripleSet* triple_set, SingleTriple* key){
	Key_TripleSet insert_key = {.val = key};
	int err = ht_erase(triple_set, &insert_key);
	return err;
}

static void fprintf_term(FILE* stream, const char* value, TERMTYPE type, const char* suffix)
{
	switch(type){
		case HELPER_LIB_GRAPH_URI:
			fprintf(stream, "<%s>", value);
			return;
		case HELPER_LIB_GRAPH_BNODE:
			fprintf(stream, "_:%s", value);
			return;
		case HELPER_LIB_GRAPH_TYPED_LITERAL:
			fprintf(stream, "\"%s\"", value);
			if (suffix != NULL){
				fprintf(stream, "^^<%s>", suffix);
			}
			return;
		case HELPER_LIB_GRAPH_LANG_LITERAL:
			fprintf(stream, "\"%s\"@%s", value, suffix);
			return;
	}
}

static void fprintf_triple(FILE* stream, SingleTriple* triple){
	fprintf(stderr, "(");
	fprintf_term(stderr, triple->subject_value,
			triple->subject_type, NULL);
	fprintf(stderr, ", ");
	fprintf_term(stderr, triple->predicate_value,
			HELPER_LIB_GRAPH_URI, NULL);
	fprintf(stderr, ", ");
	fprintf_term(stderr, triple->object_value,
			triple->object_type, triple->object_suffix);
	fprintf(stderr, ")");
}

static bool _iter_print_triple(Key_TripleSet* key, Key_TripleSet*, void*)
{
	SingleTriple* triple = key->val;
	fprintf_triple(stderr, triple);
	fprintf(stderr, "\n");
	return true;
}

static bool _iter_subtract_TripleSet(Key_TripleSet* key, Key_TripleSet*,
					TripleSet* cmp_set)
{
	SingleTriple* triple = key->val;
	int err = remove_TripleSet(cmp_set, triple);
	switch (err){
		case HT_SUCCESS:
			return true;
		case HT_ERROR:
		default:
			fprintf(stderr, "triple ");
			fprintf_triple(stderr, triple);
			fprintf(stderr, " is missing in second graph\n");
			return false;
	}
}

/**
 * Ensures equality but frees both TripleSets.
 */
static bool equal_and_free_TripleSet(TripleSet* first, TripleSet* second){
	bool node_is_missing, ret;
	fprintf(stderr, "in first:\n");
	ht_iterate(first, NULL, (ht_iterate_t) _iter_print_triple);
	fprintf(stderr, "in second:\n");
	ht_iterate(second, NULL, (ht_iterate_t) _iter_print_triple);
	node_is_missing = ht_iterate(first, second,
			(ht_iterate_t) _iter_subtract_TripleSet);
	if (node_is_missing){
		ret = false;
	} else {
		ret = ht_is_empty(second);
	}
	if(!ret){
		fprintf(stderr, "missing triples in first graph:\n");
		ht_iterate(second, NULL,
				(ht_iterate_t) _iter_print_triple);
	}
	destroy_TripleSet(first);
	destroy_TripleSet(second);
	return ret;
}

typedef struct startColorInEdgeList {
	const char* subject_value;
	const char* predicate_value;
	struct startColorInEdgeList *next;
} StartColorInEdgeList;

typedef struct startColorOutEdgeList {
	const char* predicate_value;
	const char* object_value;
	TERMTYPE object_type;
	const char* object_suffix;
	struct startColorOutEdgeList *next;
} StartColorOutEdgeList;

/**
 * Description of edges of a BNode, that arent connected to another BNode.
 * These edges determine the starting color of a BNode.
 */
typedef struct startColorStableEdges {
	StartColorInEdgeList *in;
	StartColorOutEdgeList *out;
} StartColorStableEdges;

typedef struct startColorInfo {
	BNodeDescription *bnode;
	//doesnt contain edges between bnodes
	StartColorStableEdges edges;
	//used to store listlike
	struct startColorInfo *next;
	EdgePartList *in_edges;
	EdgePartList *out_edges;
} StartColorInfo;
static void free_StartColorInfo(StartColorInfo* x);
static StartColorInfo* new_StartColorInfo(const char* bnodeid, GRAPH_ID graph_id, StartColorInfo* next);

/*
typedef HashTable StartColorTable;
typedef struct key_StartColorTable {
	const char* val;
} Key_StartColorTable;
typedef struct value_StartColorTable {
	StartColorInfo* val;
} Value_StartColorTable;
static size_t hash_StartColorTable(Key_StartColorTable* key, size_t);
static int compare_StartColorTable(Key_StartColorTable* first, Key_StartColorTable* second, size_t);
static StartColorInfo* get_StartColorTable(StartColorTable* table, const char* key);
static int insert_StartColorTable(StartColorTable* table, const char* key, StartColorInfo* value);
*/

/**
 * Map StartColorStartingEdges to MyColor
 */
typedef HashTable SCSE2Color;
typedef struct key_SCSE2Color {
	StartColorStableEdges* val;
} Key_SCSE2Color;
typedef struct value_SCSE2Color {
	MyColor* val;
} Value_SCSE2Color;
static size_t hash_SCSE2Color(Key_SCSE2Color* key, size_t);
static int compare_SCSE2Color(Key_SCSE2Color* first, Key_SCSE2Color* second, size_t);

/**
 * renamed from StartColorInfo
 */
typedef struct bnodeId2StartColorInfo {
	//only needed for freeing
	StartColorInfo* infolist;

	//StartColorTable bnodeid_to_startcolorinfo;
	HashTable map;
} BNodeId2StartColorInfo;

typedef const char* Key_BNodeId2StartColorInfo;
typedef StartColorInfo* Value_BNodeId2StartColorInfo;

static BNodeId2StartColorInfo* setup_BNodeId2StartColorInfo();
static void destroy_BNodeId2StartColorInfo(BNodeId2StartColorInfo* table);
static StartColorInfo* get_BNodeId2StartColorInfo(BNodeId2StartColorInfo *table, const char* bnodeid, GRAPH_ID graph_id);

static int compare_SCSE2Color(Key_SCSE2Color* first, Key_SCSE2Color* second, size_t){
	int cmp;
	StartColorInEdgeList *tmp_in_first, *tmp_in_second;
	StartColorOutEdgeList *tmp_out_first, *tmp_out_second;
	tmp_in_first = first->val->in;
	tmp_in_second = second->val->in;
	tmp_out_first = first->val->out;
	tmp_out_second = second->val->out;
	while(tmp_in_first != NULL && tmp_in_second != NULL){
		cmp = strcmp(tmp_in_first->subject_value,
				tmp_in_second->subject_value);
		if (cmp != 0) return cmp;
		cmp = strcmp(tmp_in_first->predicate_value,
				tmp_in_second->predicate_value);
		if (cmp != 0) return cmp;
		tmp_in_first = tmp_in_first->next;
		tmp_in_second = tmp_in_second->next;
	}
	if (tmp_in_first != NULL){
		return 1;
	} else if (tmp_in_second != NULL){
		return -1;
	}
	while(tmp_out_first != NULL && tmp_out_second != NULL){
		cmp = strcmp(tmp_out_first->predicate_value,
				tmp_out_second->predicate_value);
		if (cmp != 0) return cmp;
		cmp = strcmp(tmp_out_first->object_value,
				tmp_out_second->object_value);
		if (cmp != 0) return cmp;
		cmp = tmp_out_first->object_type - tmp_out_second->object_type;
		if (cmp != 0) return cmp;
		if (tmp_out_first->object_suffix != NULL
				&& tmp_out_second->object_suffix != NULL){
			cmp = strcmp(tmp_out_first->object_suffix,
				tmp_out_second->object_suffix);
			if (cmp != 0) return cmp;
		} else if (tmp_out_first->object_suffix != NULL) {
			return 1;
		} else if (tmp_out_second->object_suffix != NULL) {
			return -1;
		}
		tmp_out_first = tmp_out_first->next;
		tmp_out_second = tmp_out_second->next;
	}
	if (tmp_out_first != NULL){
		return 1;
	} else if (tmp_out_second != NULL){
		return -1;
	}
	return 0;
}

static size_t hash_SCSE2Color(Key_SCSE2Color* key, size_t){
	size_t hash = 5381;
	StartColorInEdgeList *tmp_in;
	StartColorOutEdgeList *tmp_out;
	tmp_in = key->val->in;
	while(tmp_in != NULL){
		hash = extend_hash_str(hash, tmp_in->subject_value);
		hash = extend_hash_str(hash, tmp_in->predicate_value);
		tmp_in = tmp_in->next;
	}
	tmp_out = key->val->out;
	while(tmp_out != NULL){
		hash = extend_hash_str(hash, tmp_out->predicate_value);
		hash = extend_hash_str(hash, tmp_out->object_value);
		hash = ((hash << 5) + hash) + tmp_out->object_type;
		if (tmp_out->object_suffix != NULL){
			hash = extend_hash_str(hash, tmp_out->object_suffix);
		}
		tmp_out = tmp_out->next;
	}
	return hash;
}

/** renamed from hash_StartColorTable */
static size_t hash_BNodeId2StartColorInfo(Key_BNodeId2StartColorInfo* key, size_t){
	size_t hash = 5381;
	hash = extend_hash_str(hash, *key);
	return hash;
}

/** renamed from compare_StartColorTable */
static int compare_BNodeId2StartColorInfo(Key_BNodeId2StartColorInfo* first, Key_BNodeId2StartColorInfo* second, size_t)
{
	return strcmp(*first, *second);
}

static bool iter_BNodeId2StartColorInfo_free_StartColorInfo(
		Key_BNodeId2StartColorInfo* key,
		Value_BNodeId2StartColorInfo* value,
		void*)
{
	StartColorInfo* x = *value;
	free_StartColorInfo(x);
	return true;
}

static void destroy_BNodeId2StartColorInfo(BNodeId2StartColorInfo* table){
	ht_iterate_remove(&(table->map), NULL,
			(ht_iterate_t) iter_BNodeId2StartColorInfo_free_StartColorInfo);
	ht_destroy(&table->map);
	free(table);
}

static BNodeId2StartColorInfo* setup_BNodeId2StartColorInfo(){
	BNodeId2StartColorInfo* new = malloc(sizeof(BNodeId2StartColorInfo));
	new->infolist = NULL;

	ht_setup(&(new->map),
			sizeof(Key_BNodeId2StartColorInfo),
			sizeof(Value_BNodeId2StartColorInfo),
			INITIAL_HASHTABLE_SIZE);
	new->map.hash = (hash_t) hash_BNodeId2StartColorInfo;
	new->map.compare = (comparison_t) compare_BNodeId2StartColorInfo;
	return new;
}


/** renamed from insert_StartColorTable */
static int insert_BNodeId2StartColorInfo(BNodeId2StartColorInfo* table, const char* key, StartColorInfo* value)
{
	Key_BNodeId2StartColorInfo insert_key = key;
	Value_BNodeId2StartColorInfo insert_value = value;
	return ht_insert(&(table->map), &insert_key, &insert_value);
}

static void free_StartColorInfo(StartColorInfo* x){
	StartColorInEdgeList *current_in, *next_in;
	StartColorOutEdgeList *current_out, *next_out;
	current_in = x->edges.in;
	while(current_in != NULL){
		next_in = current_in->next;
		free(current_in);
		current_in = next_in;
	}
	current_out = x->edges.out;
	while(current_out != NULL){
		next_out = current_out->next;
		free(current_out);
		current_out = next_out;
	}
	free(x);
}

static StartColorInfo* new_StartColorInfo(const char* bnodeid, GRAPH_ID graph_id, StartColorInfo* next){
	StartColorInfo* info = malloc(sizeof(StartColorInfo));
	info->bnode = malloc(sizeof(BNodeDescription));
	info->bnode->bnode_id = bnodeid;

	info->in_edges = NULL;
	info->out_edges = NULL;
	info->bnode->graph_id = graph_id;
	info->edges.in = NULL;
	info->edges.out = NULL;
	info->next = next;
	return info;
}

static StartColorInfo* get_BNodeId2StartColorInfo(BNodeId2StartColorInfo *table, const char* bnodeid, GRAPH_ID graph_id)
{
	Key_BNodeId2StartColorInfo key = bnodeid;
	Value_BNodeId2StartColorInfo *val1;
	StartColorInfo *info;
	if (table == NULL) return NULL;
	val1 = ht_lookup(&(table->map), &key);
	if (val1 != NULL){
		return *val1;
	}
	info = new_StartColorInfo(bnodeid, graph_id, table->infolist);
	insert_BNodeId2StartColorInfo(table, bnodeid, info);
	table->infolist = info;
	return info;
}

static int compare_bubble_once_StartColorOutEdge(StartColorOutEdgeList* first, StartColorOutEdgeList* second){
	int ret;
	ret = strcmp(first->predicate_value, second->predicate_value);
	if (ret != 0) return ret;
	ret = first->object_type - second->object_type;
	if (ret != 0) return ret;
	ret = strcmp(first->object_value, second->object_value);
	if (ret != 0) return ret;
	if (first->object_suffix != NULL){
		if (second->object_suffix != NULL){
			ret = strcmp(first->object_suffix, second->object_suffix);
		} else {
			ret = -1;
		}
	} else {
		if (second->object_suffix != NULL){
			ret = 1;
		} else {
			ret = 0;
		}
	}
	return ret;
}

static StartColorOutEdgeList *bubble_once_StartColorOutEdge(StartColorOutEdgeList *first){
	StartColorOutEdgeList fake_start = {.next = first};
	StartColorOutEdgeList *nextnext, *last = &fake_start,
			  *current = first,
			  *next = first->next;
	while (next != NULL){
		nextnext = next->next;
		if (0 < compare_bubble_once_StartColorOutEdge(current, next)){
			last->next = next;
			current->next = nextnext;
			next->next = current;
			last = next;
			next = nextnext;
		} else {
			last = current;
			current = next;
			next = nextnext;
		}
	}
	return fake_start.next;
}

static int compare_bubble_once_StartColorInEdge(StartColorInEdgeList* first, StartColorInEdgeList* second){
	int ret = strcmp(first->subject_value, second->subject_value);
	if (ret != 0) return ret;
	return strcmp(first->predicate_value, second->predicate_value);
}

static StartColorInEdgeList *bubble_once_StartColorInEdge(StartColorInEdgeList *first){
	StartColorInEdgeList fake_start = {.next = first};
	StartColorInEdgeList *nextnext, *last = &fake_start,
			  *current = first,
			  *next= first->next;
	while (next != NULL){
		nextnext = next->next;
		if (0 < compare_bubble_once_StartColorInEdge(current, next)){
			last->next = next;
			current->next = nextnext;
			next->next = current;
			last = next;
			next = nextnext;
		} else {
			last = current;
			current = next;
			next = nextnext;
		}
	}
	return fake_start.next;
}

/**
 * Add out edges to bnode info in scbn, needed for starting color.
 */
static void add_out_edge_StartColorInfo(StartColorInfo* info, SingleTriple* triple){
	StartColorOutEdgeList *new_edge = malloc(sizeof(StartColorOutEdgeList));
	new_edge->predicate_value = triple->predicate_value;
	new_edge->object_value = triple->object_value;
	new_edge->object_type = triple->object_type;
	new_edge->object_suffix = triple->object_suffix;
	new_edge->next = info->edges.out;
	info->edges.out = bubble_once_StartColorOutEdge(new_edge);
}

/**
 * Add in edges to bnode info in scbn, needed for starting color.
 */
static void add_in_edge_StartColorInfo(StartColorInfo* info, SingleTriple* triple){
	StartColorInEdgeList *new_edge = malloc(sizeof(StartColorInEdgeList));
	new_edge->subject_value = triple->subject_value;
	new_edge->predicate_value = triple->predicate_value;
	new_edge->next = info->edges.in;
	info->edges.in = bubble_once_StartColorInEdge(new_edge);
}

/**
 * Establish a link between 2 bnodes, needed for refinement.
 */
static void connect_StartColorInfo(StartColorInfo *subject, const char* predicate, StartColorInfo *object)
{
	subject->out_edges = insert_EdgePartList(subject->out_edges, predicate, object->bnode);
	object->in_edges = insert_EdgePartList(object->in_edges, predicate, subject->bnode);
}

static EdgePartList* insert_EdgePartList(EdgePartList *rest, const char* predicate, BNodeDescription *target)
{
	EdgePartList* new_list = malloc(sizeof(EdgePartList));
	new_list->next = rest;
	new_list->first = malloc(sizeof(BNodeEdge));
	new_list->first->property_value = predicate;
	new_list->first->target = target;
	return new_list;
	/*
	int cmp;
	bool new_predicate = true;
	EdgePartList *last, *next;
	EdgePartList fake_start = {
		.property_id = 0,
		.property_value = predicate,
		.next = start
	};
	EdgePartList* new = malloc(sizeof(EdgePartList));
	new->property_id = 0;
	new->property_value = predicate;
	new->target = target;
	new->next = NULL;
	if (start == NULL){
		return new;
	}
	last = &fake_start;
	next = start;
	cmp = 1;
	while (cmp > 0 && last->next != NULL){
		next = last->next;
		cmp = strcmp(new->property_value, next->property_value);
		if (cmp == 0){
			new->property_id = next->property_id;
			new_predicate = false;
			cmp = new->target->graph_id - next->target->graph_id;
			if (cmp == 0){
				cmp = strcmp(new->target->bnode_id,
						next->target->bnode_id);
			}
		}
		if (cmp > 0){
			next = next->next;
			last = next;
		}
	}
	if (cmp == 0){
		free(new);
	} else {
		last->next = new;
		new->next = next;
		if (new_predicate){
			increase_property_id_EdgePartList(next);
		}
	}
	return fake_start.next;
	*/
}

static int compare_BNodeEdge(const BNodeEdge* first, const BNodeEdge* second){
	int cmp = strcmp(first->property_value, second->property_value);
	if (cmp != 0) return cmp;
	cmp = first->target->last_color - second->target->last_color;
	if (cmp != 0) return cmp;
	cmp = first->target->last_color->index - second->target->last_color->index;
	return cmp;
}

/**
 * Uses bubblesort
 */
/*
static EdgePartList* sort_EdgePartList(EdgePartList* start){
	if (start == NULL) return NULL;
	int l = 0;
	bool swap = false;
	EdgePartList fake_start = {.next = start};
	EdgePartList *last = &fake_start, *current = start, *next = start->next;
	while (next != NULL){
		if (compare_EdgePartList(current, next) < 0){
			last->next = next;
			current->next = next->next;
			next->next = current;
			swap = true;
		}
		last = current;
		current = next;
		next = next->next;
		l++;
	}
	for (int i = l-1; i>0; i--){
		swap = false;
		last = &fake_start; current = start; next = start->next;
		for (int j = 0; j<i; j++){
			if (compare_EdgePartList(current, next) < 0){
				last->next = next;
				current->next = next->next;
				next->next = current;
				swap = true;
			}
			last = current;
			current = next;
			next = next->next;
		}
		if (!swap) break;
	}
	return fake_start.next;
}
*/

static void gather_color_information_and_stable_edges(TripleList* start, TripleSet *triple_set, BNodeId2StartColorInfo* scbn, GRAPH_ID graph_id);

struct generate_starting_ColorPalette {
	ColorPalette *colors;
	SCSE2Color scse2color;
};

static ColorPalette* new_ColorPalette(){
	ColorPalette* new = malloc(sizeof(ColorPalette));
	new->first = NULL;
	//new->first_nontrivial = NULL;
	new->last = NULL;
	return new;
}

static MyColor* new_Color(ColorPalette* color_palette, BNodeDescription* bnode){
	MyColor* new_color = malloc(sizeof(MyColor));
	MyColor* last = color_palette->last;
	BNodeDescriptionList* new_bnode_list;
	new_bnode_list = malloc(sizeof(BNodeDescriptionList));

	if (last == NULL){
		new_color->index = 0;
		color_palette->first = new_color;
		//color_palette->first_nontrivial = new_color;
	} else {
		new_color->index = last->index + 1;
		color_palette->last->next = new_color;
		//color_palette->last->nontrivial_next = new_color;
	}
	new_color->next = NULL;
	//new_color->nontrivial_next = NULL;
	color_palette->last = new_color;
	new_bnode_list->first = bnode;
	new_bnode_list->next = NULL;
	new_color->bnode_list = new_bnode_list;
	new_color->parent = color_palette;
	return new_color;
}

static void add_bnode_Color(MyColor* color, BNodeDescription* bnode) {
	BNodeDescriptionList *tmp, *new;
	new = malloc(sizeof(BNodeDescriptionList));
	new->first = bnode;
	new->next = NULL;
	tmp = color->bnode_list;
	if (tmp != NULL){
		while (tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = new;
	} else {
		color->bnode_list = new;
	}
}

static MyColor* get_SCSE2Color(SCSE2Color* table, StartColorStableEdges* key){
	Key_SCSE2Color lookup_key = {.val = key};
	Value_SCSE2Color* value;
	value = ht_lookup(table, &lookup_key);
	if (value != NULL){
		return value->val;
	} else {
		return NULL;
	}
}
static int insert_SCSE2Color(SCSE2Color* table, StartColorStableEdges* key, MyColor* value)
{
	int err;
	Key_SCSE2Color lookup_key = {.val = key};
	Value_SCSE2Color lookup_value = {.val = value};
	err = ht_insert(table, &lookup_key, &lookup_value);
	return err;
}

/**
 * consume Edgepartlist
 */
static void create_BNodeEdges(EdgePartList *linked_list, BNodeEdge ***target, size_t *target_length)
{
	size_t i = 0;
	EdgePartList *tmpnext;
	BNodeEdge **new_list;
	if (linked_list == NULL){
		*target = NULL;
		*target_length = 0;
		return;
	}
	for (EdgePartList *x = linked_list; x != NULL; x = x->next){
		i++;
	}
	new_list = calloc(i, sizeof(BNodeEdge));
	*target = new_list;
	*target_length = i;
	i = 0;
	for (EdgePartList *x = linked_list; x != NULL; x = x->next){
		new_list[i] = x->first;
		i++;
	}
	for (EdgePartList *x = linked_list; x != NULL; x = tmpnext){
		tmpnext = x->next;
		free(x);
	}
}

static bool generate_colors_from_scbn_iter(
		Key_BNodeId2StartColorInfo*,
		Value_BNodeId2StartColorInfo* value,
		struct generate_starting_ColorPalette* user)
{
	MyColor* start_color;
	StartColorInfo* info = *value;
	start_color = get_SCSE2Color(&user->scse2color, &(info->edges));
	if (start_color == NULL){
		start_color = new_Color(user->colors, info->bnode);
		insert_SCSE2Color(&(user->scse2color),
					&(info->edges), start_color);
	} else {
		add_bnode_Color(start_color, info->bnode);
	}
	info->bnode->last_color = start_color;
	create_BNodeEdges(info->in_edges, &info->bnode->in, &info->bnode->in_length);
	info->in_edges = NULL;
	create_BNodeEdges(info->out_edges, &info->bnode->out, &info->bnode->out_length);
	info->out_edges = NULL;
	return true;
}

static void setup_SCSE2Color(SCSE2Color* table){
	ht_setup(table, sizeof(Key_SCSE2Color),
			sizeof(Value_SCSE2Color), INITIAL_HASHTABLE_SIZE);
	table->hash = (hash_t) hash_SCSE2Color;
	table->compare = (comparison_t) compare_SCSE2Color;
}


static ColorPalette* generate_starting_ColorPalette(
		BNodeId2StartColorInfo* info_first_graph,
		BNodeId2StartColorInfo* info_second_graph)
{
	struct generate_starting_ColorPalette user = {
		.colors = new_ColorPalette(),
	};
	setup_SCSE2Color(&user.scse2color);
	ht_iterate(&info_first_graph->map, &user,
			(ht_iterate_t) generate_colors_from_scbn_iter);
	ht_iterate(&info_second_graph->map, &user,
			(ht_iterate_t) generate_colors_from_scbn_iter);
	ht_destroy(&user.scse2color);
	return user.colors;
}


static ColorPalette* create_initial_color_and_check_trivial_edges(
		TripleList* first, TripleList* second)
{
	ColorPalette* all_colors;
	TripleSet* triples_first = setup_TripleSet();
	TripleSet* triples_second = setup_TripleSet();
	BNodeId2StartColorInfo* scbn_first = setup_BNodeId2StartColorInfo();
	BNodeId2StartColorInfo* scbn_second = setup_BNodeId2StartColorInfo();
	gather_color_information_and_stable_edges(first,
						triples_first, scbn_first,
						ID_FIRST_GRAPH);
	gather_color_information_and_stable_edges(second,
						triples_second, scbn_second,
						ID_SECOND_GRAPH);
	if(equal_and_free_TripleSet(triples_first, triples_second)){
		all_colors = generate_starting_ColorPalette(scbn_first, scbn_second);
	} else {
		all_colors = NULL;
	}
	destroy_BNodeId2StartColorInfo(scbn_first);
	destroy_BNodeId2StartColorInfo(scbn_second);
	return all_colors;
}

static void gather_color_information_and_stable_edges(TripleList* start, TripleSet *triple_set, BNodeId2StartColorInfo* scbn, GRAPH_ID graph_id)
{
	StartColorInfo *tmpinfo, *tmpinfo2;
	SingleTriple *triple;
	for (TripleList* tmp = start; tmp != NULL; tmp = tmp->rest){
		triple = &(tmp->first);
		if (triple->subject_type == HELPER_LIB_GRAPH_BNODE
				&& triple->object_type == HELPER_LIB_GRAPH_BNODE)
		{
			tmpinfo = get_BNodeId2StartColorInfo(scbn,
					triple->subject_value, graph_id);
			tmpinfo2 = get_BNodeId2StartColorInfo(scbn,
					triple->object_value, graph_id);
			connect_StartColorInfo(tmpinfo,
					triple->predicate_value, tmpinfo2);
		} else if (triple->subject_type == HELPER_LIB_GRAPH_BNODE) {
			tmpinfo = get_BNodeId2StartColorInfo(scbn,
					triple->subject_value, graph_id);
			add_out_edge_StartColorInfo(tmpinfo, triple);
		} else if (triple->object_type == HELPER_LIB_GRAPH_BNODE) {
			tmpinfo = get_BNodeId2StartColorInfo(scbn,
					triple->object_value, graph_id);
			add_in_edge_StartColorInfo(tmpinfo, triple);
		} else {
			add_TripleSet(triple_set, triple);
		}
	}
}



static MyColor* get_last_color(MyColor* color){
	MyColor* x = color;
	if (color == NULL) return NULL;
	while (x->next != NULL){
		x = x->next;
	}
	return x;
}

/**
 * Compare color of two same colored nodes, by comparing all edges with
 * one another.
 * Edges are sorted here before comparing.
 *
 */
static int compare_color(BNodeDescription *first, BNodeDescription *second){
	int cmp;
	EdgePartList *edge_first, *edge_second;
	cmp = first->in_length - second->in_length;
	if (cmp != 0) return cmp;
	cmp = first->out_length - second->out_length;
	if (cmp != 0) return cmp;
	for (int i=0; i<first->in_length; i++){
		cmp = compare_BNodeEdge(first->in[i], second->in[i]);
		if (cmp != 0) return cmp;
	}
	for (int i=0; i<first->out_length; i++){
		cmp = compare_BNodeEdge(first->out[i], second->out[i]);
		if (cmp != 0) return cmp;
	}
	return 0;
}


/*
static void generate_or_assign_color(MyColor* available_colors, BNodeDescription *new)
{
	MyColor *last = available_colors;
	BNodeDescription *first_bnode;
	for (MyColor *color = available_colors->next;
			color != NULL;
			color = color->next)
	{
		first_bnode = color->bnode_list->first;
		if (first_bnode->last_color == new->last_color
				&& 0 == compare_color(first_bnode, new))
		{
			color->bnode_list = insert_BNodeDescriptionList(
							color->bnode_list, new);
		}
	}
}
*/

static int number_colors(ColorPalette* colors){
	int i = 0;
	for (MyColor *color = colors->first;
			color != NULL;
			color = color->next)
	{
		i++;
	}
	return i;
}
