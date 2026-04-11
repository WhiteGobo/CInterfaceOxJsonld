use std::ptr;
use std::os::raw::{c_char, c_void};
use std::ffi::CStr;
use oxjsonld::{JsonLdParser, JsonLdParseError};

use crate::rename_error::my_error_print;
use crate::document_loader::{
    LoadDocumentCallbackOption,
};
use crate::parser_config::{
    JSONLDConfig,
};
use crate::intern::{
    TripleHandler,
    call_hook,
    CallHookError,
};
use crate::error::{
    MyIoErr,
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
