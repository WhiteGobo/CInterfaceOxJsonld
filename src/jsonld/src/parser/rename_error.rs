use oxjsonld::JsonLdSyntaxError;

pub fn my_error_print(e: JsonLdSyntaxError) {
    use oxjsonld::JsonLdErrorCode;
    //relabel errorcodes to work as described in jsonld tests
    let code = match e.code() {
        Some(x) => x,
        None => {return;}
    };
    match code {
            JsonLdErrorCode::ProtectedTermRedefinition => {
                eprintln!("Error: protected term redefinition");
            },
            JsonLdErrorCode::InvalidTypeMapping => {
                eprintln!("Error: invalid type mapping");
            },
            JsonLdErrorCode::InvalidImportValue => {
                eprintln!("Error: invalid @import value");
            },
            JsonLdErrorCode::InvalidContextNullification => {
                eprintln!("Error: invalid context nullification");
            },
            JsonLdErrorCode::InvalidIncludedValue => {
                eprintln!("Error: invalid @included value");
            },
            JsonLdErrorCode::InvalidIriMapping => {
                eprintln!("Error: invalid IRI mapping");
            },
            JsonLdErrorCode::InvalidScopedContext => {
                eprintln!("Error: invalid scoped context");
            },
            JsonLdErrorCode::KeywordRedefinition => {
                eprintln!("Error: keyword redefinition");
            },
            JsonLdErrorCode::InvalidTypedValue => {
                eprintln!("Error: invalid typed value");
            },
            JsonLdErrorCode::InvalidNestValue => {
                eprintln!("Error: invalid @nest value");
            },
            JsonLdErrorCode::InvalidTermDefinition => {
                eprintln!("Error: invalid term definition");
            },
            JsonLdErrorCode::InvalidDefaultLanguage => {
                eprintln!("Error: invalid default language");
            },
            JsonLdErrorCode::InvalidValueObject => {
                eprintln!("Error: invalid value object");
            },
            JsonLdErrorCode::InvalidVocabMapping => {
                eprintln!("Error: invalid vocab mapping");
            },
            JsonLdErrorCode::InvalidBaseIri => {
                eprintln!("Error: invalid base IRI");
            },
            JsonLdErrorCode::InvalidLocalContext => {
                eprintln!("Error: invalid local context");
            },
            JsonLdErrorCode::InvalidVersionValue => {
                eprintln!("Error: invalid @version value");
            },
            JsonLdErrorCode::InvalidReverseProperty => {
                eprintln!("Error: invalid reverse property");
            },
            JsonLdErrorCode::InvalidBaseDirection => {
                eprintln!("Error: invalid base direction");
            },
            JsonLdErrorCode::InvalidPropagateValue => {
                eprintln!("Error: invalid @propagate value");
            },
            JsonLdErrorCode::LoadingRemoteContextFailed => {
                eprintln!("Error: loading remote context failed");
            },
            //JsonLdErrorCode:: => {
            //    eprintln!("Error: ");
            //},
            _ => {},
    }
}
