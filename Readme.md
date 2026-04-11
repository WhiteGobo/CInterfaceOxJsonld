# cinterface-oxjsonld - Small jsonld parser as interface library for oxjsonld

Implements capabilities to parse jsonld in C via rusts oxjsonld library.
This is a early version and i intend to rename the headerfile
and the provided functions.

## example:
See `jsonld.h` for all available functions.

```C
#include <stdio.h>
#include <stdlib.h>
#include "jsonld.h"

static int8_t my_triplehandler(
                const char* subject, uint8_t subject_type,
                const char* predicate,
                const char* object, const char* object_suffix,
                uint8_t object_type,
                const char* graph_id, uint8_t graph_type,
                void* user)
{
    printf("%s %s %s", subj_v, pred_v, obj_v);
    return 0;
}

int main(int argc, char *argv[]){
    char* base_uri = "http://example.com/baseuri/";
	const char *jsonlddata = "..."; //example jsonld data
	JSONLDConfig* config = JSONLDConfig_set_baseiri(config, base_uri);
    void* user == NULL; //triplehandler context
    if (config == NULL) exit(1);
	config = JSONLDConfig_enable_LoadDocumentCallback_over_http(config);
    if (config == NULL) exit(1);
	int err = parse_jsonld(jsonlddata, my_triplehandler, user, config);
    exit(err);
}
```

## Doesnt work:

I have disabled quite few testcases provided by jsonld. These are the problems as far as i know:

- Cannot parse Blank nodes as predicate.
- Has problems with context placed relative to base uri.
    Eg `"@context": "same-directory.jsonld"` fails,
    but `"@context": "other_directory/context.jsonld"` works.


## Dependencies

All dependencies and subdependencies used during build, will be gathered
in the produced `{BUILD}/README.md` via cargo-license.

Direct dependencies in produced library:

oxjsonld: https://crates.io/crates/oxjsonld
    Licensed under MIT [1] or Apache-2.0 [3]
oxrdf: https://crates.io/crates/oxrdf
    Licensed under MIT [1] or Apache-2.0 [3]


Used in testing and build process:

petgraph: https://crates.io/crates/petgraph
    Licensed under MIT [1] or Apache-2.0 [3]
cwalk: https://github.com/likle/cwalk.git
    Licensed under MIT [2]
cargo-license: https://crates.io/crates/cargo-license
    Licensed under MIT [1]


    [1] https://choosealicense.com/licenses/mit/
    [2] https://github.com/likle/cwalk/blob/master/LICENSE.md
    [3] https://choosealicense.com/licenses/apache-2.0/
