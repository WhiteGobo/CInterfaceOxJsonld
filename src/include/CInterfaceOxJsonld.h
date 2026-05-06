#pragma once
/**
 * more or less generated with cbindgen on lib.rs
 */

#include <stdio.h>
#include <stdint.h>

#ifndef RDF_TERMTYPE_DEFINED
#define RDF_TERMTYPE_DEFINED
typedef enum {
        URI = 0,
        BNODE = 1,
        TYPEDLITERAL = 2,
        LANGLITERAL = 3
} TERMTYPE;
#endif


typedef struct jsonldConfig JSONLDConfig;

/*
 * Use TERMTYPE for subject_type, object_type and graph_type.
 * If graphid is NULL, the default graph is used.
 */
typedef int8_t TripleHandler(
                const char* subject, uint8_t subject_type,
                const char* predicate,
                const char* object, const char* object_suffix,
                uint8_t object_type,
                const char* graphid, uint8_t graph_type,
                void* user);

#ifdef __cplusplus
//namespace: CInterfaceOxJsonld
extern "C" {
#endif

void free_JSONLDConfig(JSONLDConfig*);
JSONLDConfig* JSONLDConfig_set_baseiri(JSONLDConfig*, const char*);
JSONLDConfig* JSONLDConfig_use_processing_mode_10(JSONLDConfig*);
JSONLDConfig* JSONLDConfig_use_processing_mode_11(JSONLDConfig*);
JSONLDConfig* JSONLDConfig_enable_LoadDocumentCallback_over_http(JSONLDConfig*);
JSONLDConfig* JSONLDConfig_enable_LoadDocumentCallback_for_localfiles(JSONLDConfig*);
JSONLDConfig* JSONLDConfig_enable_LoadDocumentCallback_for_internet(JSONLDConfig*);
JSONLDConfig *JSONLDConfig_enable_LoadDocumentCallback_for_relativefiles(
						JSONLDConfig *config,
						const char *baseuri_c,
						const char *basepath_c);

int64_t parse_jsonld(const char *input, TripleHandler hook, void* hook_data, JSONLDConfig* config);


#ifdef __cplusplus
}
#endif
