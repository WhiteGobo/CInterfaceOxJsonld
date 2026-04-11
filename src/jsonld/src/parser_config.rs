use std::io::Read;
use oxjsonld::{ReaderJsonLdParser,};
use std::error::Error;
use std::fmt;
use oxjsonld::{JsonLdLoadDocumentOptions, JsonLdRemoteDocument, JsonLdParser};
use oxiri::IriParseError;

use crate::error::MYERR;
use crate::document_loader::{
    LoadDocumentCallbackOption,
    call_document_loader,
};


pub struct JSONLDConfig {
    pub callbacks: Vec<LoadDocumentCallbackOption>,
    pub base_iri: Option<String>,
}


impl JSONLDConfig {
    pub fn new() -> Self {
        JSONLDConfig {
            callbacks: Vec::new(),
            base_iri: None,
        }
    }

    pub fn decorate_parser(&self, mut parser: JsonLdParser)
        -> Result<JsonLdParser, IriParseError>
    {
        match &self.base_iri {
            Some(base_iri) => {
                parser = match parser.with_base_iri(base_iri){
                    Ok(x) => x,
                    Err(e) => {return Err(e);},
                }
            },
            None => {},
        }
        Ok(parser)
    }

    pub fn decorate_reader<'a, R: Read>(
        &'a self, mut parser: ReaderJsonLdParser<R>)
        -> ReaderJsonLdParser<R>
    {
        parser = self.decorate_with_callback(parser);
        parser
    }

    fn decorate_with_callback<'a, R: Read>(
        &'a self, parser: ReaderJsonLdParser<R>)
        -> ReaderJsonLdParser<R>
    {
        let readerlist = self.callbacks.clone();
        parser.with_load_document_callback(
            move |url, options| {call_document_loader(url, options, &readerlist)}
        )
    }
}
