/*
 * https://www.w3.org/TR/turtle/#grammar-production-IRIREF
 */

%Start iriref doublequoted singlequoted bnode
%{
#include "term.h"
#include "nquads_parser.qq.h"

typedef struct releaseList {
	void* item;
	struct releaseList* next;
} ReleaseList;

int yyerror(char *s);

char* gen_string(const char*, size_t len);
void nquads_scanner_tidy_up();

ReleaseList *release_list = NULL;
%}

letter [a-zA-Z]
digit [0-9]
alfa [a-zA-Z0-9_]
whitesp [ \t]


hex [a-zA-Z0-9]
esc [_~.-!$&'()*+,;=/?#@%]
notiri [#x00-#x20<>"{}|^`\\]
pncharsbase [A-Z] | [a-z] | [#x00C0-#x00D6] | [#x00D8-#x00F6] | [#x00F8-#x02FF] | [#x0370-#x037D] | [#x037F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]


irirefval [^\x00-\x20<>"{}|^`\\]+
langstring [a-zA-Z\-]+
echar ([\\][tbnrf"'\\])
uchar ("\\u"hex hex hex hex)
singlequotedstring ([^\x27\x5C\xA\xD])+
doublequotedstring ([^\x22\x5C\xA\xD])+
bnodeval [A-Za-z0-9_-]+


%%
<INITIAL>"<" BEGIN(iriref);
<iriref>{irirefval} {yylval.string_val = gen_string(yytext, yyleng); return IRIREF;}
<iriref>">" BEGIN(INITIAL);

<INITIAL>"\"" BEGIN(doublequoted);
<doublequoted>{doublequotedstring} {yylval.string_val = gen_string(yytext, yyleng); return STRING;}
<doublequoted>"\"" BEGIN(INITIAL);
<INITIAL>"\'" BEGIN(singlequoted);
<singlequoted>{singlequotedstring} {yylval.string_val = gen_string(yytext, yyleng); return STRING;}
<singlequoted>"\'" BEGIN(INITIAL);
<INITIAL>"^""^" {return DOUBLECARET;}
<INITIAL>"@" {return ATSIGN;}
<INITIAL>{langstring} {yylval.string_val = gen_string(yytext, yyleng); return STRING;}

<INITIAL>"_"":" BEGIN(bnode);
<bnode>{bnodeval} {yylval.string_val = gen_string(yytext, yyleng); return BNODE;}
<bnode>{whitesp} BEGIN(INITIAL);
<INITIAL>{whitesp}* {}
<INITIAL>"\." {return END_TRIPLE;}
"\n" {yylineno++;}

%%

int yywrap() { /* need this to avoid link problem */
	return 1;
}

char* gen_string(const char* input, size_t length){
	ReleaseList* tmp = malloc(sizeof(ReleaseList));
	char* ret = malloc(length+1);
	memcpy(ret, input, length);
	ret[length] = '\0';
	tmp->item = ret;
	tmp->next = release_list;
	release_list = tmp;
	return ret;
}

static void delete_string_stack(){
	ReleaseList *next, *current = release_list;
	while (current != NULL){
		next = current->next;
		free(current->item);
		free(current);
		current = next;
	}
	release_list = NULL;

}

void nquads_scanner_tidy_up(){
	delete_string_stack();
	//See https://stackoverflow.com/questions/43671389/minimal-bison-flex-generated-code-has-memory-leak
	yy_delete_buffer(YY_CURRENT_BUFFER);
	yylex_destroy();
}
