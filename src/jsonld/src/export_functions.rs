use std::ptr;
use std::os::raw::{c_char, c_uchar, c_void};
use std::ffi::CStr;
use oxjsonld::{JsonLdParser, JsonLdParseError};

use crate::parser::rename_error::my_error_print;
use crate::parser::document_loader::{
    LoadDocumentCallbackOption,
};
use crate::parser::parser_config::{
    JSONLDConfig,
};
use crate::parser::intern::{
    TripleHandler,
    call_hook,
    CallHookError,
};
use crate::error::{
    MyIoErr,
};

use crate::serializer::config::{
    JSONLDSerializer,
};
use crate::serializer::genterms::{
    generate_IdentifiedNode, generate_IRI, generate_Term, generate_Graph,
};



#[unsafe(no_mangle)]
pub extern "C" fn free_JSONLDConfig(config: *mut JSONLDConfig){
    if !config.is_null(){
        unsafe { let _ = Box::from_raw(config); }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn JSONLDConfig_set_baseiri(
    mut config: *mut JSONLDConfig, base_uri: *const c_char)
    -> *mut JSONLDConfig
{
    if config.is_null() {
        let x = JSONLDConfig::new();
        let mybox = Box::new(x);
        config = Box::into_raw(mybox)
    }
    let base_iri_str: &str = match unsafe {CStr::from_ptr(base_uri)}.to_str(){
        Ok(x) => x,
        Err(_) => {
            return ptr::null_mut();
        },
    };
    unsafe{
        (*config).base_iri = Some(base_iri_str.to_string());
    }
    config
}

#[unsafe(no_mangle)]
pub extern "C" fn JSONLDConfig_use_processing_mode_10(
    mut config: *mut JSONLDConfig
    ) -> *mut JSONLDConfig
{
    if config.is_null() {
        let x = JSONLDConfig::new();
        let mybox = Box::new(x);
        config = Box::into_raw(mybox)
    }
    config
}


#[unsafe(no_mangle)]
pub extern "C" fn JSONLDConfig_use_processing_mode_11(
    mut config: *mut JSONLDConfig
    ) -> *mut JSONLDConfig
{
    if config.is_null() {
        let x = JSONLDConfig::new();
        let mybox = Box::new(x);
        config = Box::into_raw(mybox)
    }
    config
}

#[unsafe(no_mangle)]
pub extern "C" fn JSONLDConfig_enable_LoadDocumentCallback_over_http(
    mut config: *mut JSONLDConfig,
    ) -> *mut JSONLDConfig
{
    if config.is_null() {
        let x = JSONLDConfig::new();
        let mybox = Box::new(x);
        config = Box::into_raw(mybox)
    }
    let cb = LoadDocumentCallbackOption::HttpFile;
    unsafe{(*config).callbacks.push(cb);}
    config
}

#[unsafe(no_mangle)]
pub extern "C" fn JSONLDConfig_enable_LoadDocumentCallback_for_relativefiles(
    mut config: *mut JSONLDConfig,
    baseuri_c: *const c_char, basepath_c: *const c_char
    ) -> *mut JSONLDConfig
{
    let baseuri = match unsafe {CStr::from_ptr(baseuri_c)}.to_str(){
        Ok(x) => x,
        Err(_) => {return ptr::null_mut();}
    };
    let basepath = match unsafe {CStr::from_ptr(basepath_c)}.to_str(){
        Ok(x) => x,
        Err(_) => {return ptr::null_mut();}
    };
    if config.is_null() {
        let x = JSONLDConfig::new();
        let mybox = Box::new(x);
        config = Box::into_raw(mybox)
    }
    let cb = LoadDocumentCallbackOption::RelativeFile(baseuri.into(),
                                                    basepath.into());
    unsafe{(*config).callbacks.push(cb);}
    config
}


#[unsafe(no_mangle)]
pub extern "C" fn JSONLDConfig_enable_LoadDocumentCallback_for_localfiles(
    mut config: *mut JSONLDConfig
    ) -> *mut JSONLDConfig
{
    if config.is_null() {
        let x = JSONLDConfig::new();
        let mybox = Box::new(x);
        config = Box::into_raw(mybox)
    }
    unsafe{(*config).callbacks.push(LoadDocumentCallbackOption::LocalFile);}
    config
}


#[unsafe(no_mangle)]
pub extern "C" fn JSONLDConfig_enable_LoadDocumentCallback_for_internet(
    mut config: *mut JSONLDConfig
    ) -> *mut JSONLDConfig
{
    if config.is_null() {
        let x = JSONLDConfig::new();
        let mybox = Box::new(x);
        config = Box::into_raw(mybox)
    }
    config
}

fn print_parse_error(e: JsonLdParseError){
    match e {
        JsonLdParseError::Syntax(syn_err) => {
            eprintln!("{}", syn_err);
            my_error_print(syn_err);
        },
        JsonLdParseError::Io(io_err) => {
            match io_err.downcast::<MyIoErr>() {
                Ok(io_err) => {
                    eprintln!("{}", io_err);
                },
                Err(e) => {
                    eprintln!("{}", e);
                },
            }
        },
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn parse_jsonld(
    input: *const c_char,
    hook: TripleHandler,
    hook_data: *mut c_void,
    extra_config: *mut JSONLDConfig)
    -> i64
{
    use oxrdf::Quad;

    let x: &CStr = unsafe {CStr::from_ptr(input)};
    let mut parser = JsonLdParser::new();
    if !extra_config.is_null() {unsafe{
        parser = match (*extra_config).decorate_parser(parser){
            Ok(x) => x,
            Err(e) => {
                eprintln!("{}", e);
                return -1;
            }
        };
    }}
    let mut reader = parser.for_reader(x.to_bytes());

    if !extra_config.is_null() {unsafe{
        reader = (*extra_config).decorate_reader(reader);
    }}

    for triple in reader {
        let triple: Quad = match triple {
            Ok(x) => x,
            Err(e) => {
                print_parse_error(e);
                return -3;
            },
        };

        match call_hook(triple, hook, hook_data){
            Ok(()) => {},
            Err(e) => {
                eprintln!("{}", e);
                return -1;
            },
        }
    }
    return 0;
}


#[unsafe(no_mangle)]
pub extern "C" fn JSONLD_SER_start() -> *mut JSONLDSerializer
{
    let x = JSONLDSerializer::new();
    let mybox = Box::new(x);
    let config = Box::into_raw(mybox);
    config
}

#[unsafe(no_mangle)]
pub extern "C" fn JSONLD_SER_set_base_iri(
    mut config: *mut JSONLDSerializer, baseiri: *const c_char,
    ) -> *mut JSONLDSerializer
{
    if config.is_null() {
        let x = JSONLDSerializer::new();
        let mybox = Box::new(x);
        config = Box::into_raw(mybox)
    }
    config
}

#[unsafe(no_mangle)]
pub extern "C" fn JSONLD_SER_set_prefix(
    mut config: *mut JSONLDSerializer, name: *const c_char, iri: *const c_char,
    ) -> *mut JSONLDSerializer
{
    if config.is_null() {
        let x = JSONLDSerializer::new();
        let mybox = Box::new(x);
        config = Box::into_raw(mybox)
    }
    config
}

#[unsafe(no_mangle)]
pub extern "C" fn JSONLD_SER_finish(
    config: *mut JSONLDSerializer,
    ) -> *mut c_uchar
{
    if !config.is_null(){
        unsafe {
            let mut cfg = Box::from_raw(config);
            match unsafe{cfg.finish()} {
                Ok(mut x) => x.as_mut_ptr(),
                Err(_) => ptr::null_mut(),
            }
        }
    } else {
        ptr::null_mut()
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn JSONLD_SER_add(
    subject: *const c_char, subject_type: u8,
    predicate: *const c_char,
    object: *const c_char, object_suffix: *const c_char,
    object_type: u8,
    graph_id: *const c_char, graph_type: u8,
    serializer: *mut JSONLDSerializer,
    ) -> i64
{
    if serializer.is_null() {
        return -1;
    }
    let subj = match generate_IdentifiedNode(subject, subject_type, serializer) {
        Ok(x) => x,
        Err(_) => {return -2;},
    };
    let pred = match generate_IRI(predicate) {
        Ok(x) => x,
        Err(_) => {return -3;},
    };
    let obj = match generate_Term(object, object_suffix, object_type, serializer){
        Ok(x) => x,
        Err(_) => {return -4;},
    };
    let graph = match generate_Graph(graph_id, graph_type, serializer) {
        Ok(x) => x,
        Err(_) => {return -5;},
    };
    unsafe{
        (*serializer).serialize_quad(subj, pred, obj, graph);
    }
    return 0;
}
