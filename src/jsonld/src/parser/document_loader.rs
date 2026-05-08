use std::error::Error;
use oxjsonld::{JsonLdLoadDocumentOptions, JsonLdRemoteDocument, JsonLdParser};
use crate::error::{MYERR, MyIoErr};
use std::path::PathBuf;


#[derive(Clone)]
pub enum LoadDocumentCallbackOption {
    RelativeFile(String, String),
    LocalFile,
    HttpFile,
}


pub fn call_document_loader(
    url: &str, options: &JsonLdLoadDocumentOptions,
    callback_functions: &Vec<LoadDocumentCallbackOption>)
    -> Result<JsonLdRemoteDocument, Box<dyn Error + Send + Sync>>
{
    use LoadDocumentCallbackOption::{
        RelativeFile,
        LocalFile,
        HttpFile,
    };
    let mut errors = Vec::new();
    for cllbck in callback_functions {
        let result = match cllbck {
            LocalFile => {
                loaddocument_callback_for_localfiles(url, options)
            },
            RelativeFile(baseuri, basepath) => {
                loaddocument_callback_for_relative_files(url, options,
                                                    &basepath, &baseuri)
            },
            HttpFile => {
                loaddocument_callback_over_http(url, options)
            },
        };
        match result {
            Ok(x) => {return Ok(x);},
            Err(e) => {errors.push(e)},
        }
    }
    let errmsg = errors.iter().map(|e| e.to_string()).collect();
    return Err(MyIoErr::NotFound(errmsg).into());
}

fn get_existing_jsonld(basepath: &str, file: &str) -> Result<PathBuf, String>{
    use std::path::Path;
    let p = Path::new(basepath).join(file);
    if p.exists() {
        return Ok(p);
    }
    let p2 = Path::new(basepath).join(file.to_string() + ".jsonld");
    if p2.exists() {
        return Ok(p2);
    }
    return Err(p.display().to_string()+("(.jsonld)"));
}

fn loaddocument_callback_over_http(
    url: &str, options: &JsonLdLoadDocumentOptions)
    -> Result<JsonLdRemoteDocument, MYERR>
{
    use oxhttp::Client;
    use oxhttp::model::{Body, Request, Method, StatusCode, HeaderName, HeaderValue};
    use oxhttp::model::header::CONTENT_TYPE;
    use std::io::Read;

    let client = Client::new().with_redirection_limit(3);
    let request = match Request::builder().uri(url).body(Body::empty()){
        Ok(x) => x,
        Err(_) => {return Err(MYERR::HttpBody);},
    };
    let response = match client.request(request){
        Ok(x) => x,
        Err(e) => {return Err(MYERR::HttpRequest(e.into()));},
    };
    match response.status() {
        StatusCode::OK => {},
        status => {return Err(MYERR::HttpStatus(status));},
    }
    //ignore header
    match response.headers().get(CONTENT_TYPE) {
        Some(header) => match header.to_str() {
            Ok("text/html") => {},
            Ok(x) => {
                eprintln!("http request returned wrong content type {}", x);
            },
            _ => {eprintln!("http request couldnt identify content type");},
        },
        _ => {eprintln!("http request misses content type");},
    }
    let data: Vec<u8> = match response.into_body().to_vec(){
        Ok(x) => x,
        Err(e) => {return Err(MYERR::HttpBodyTransform(e.into()));},
    };
    Ok(JsonLdRemoteDocument {
        document: data,
        document_url: url.into(),
    })
}


fn loaddocument_callback_for_relative_files(
    url: &str, options: &JsonLdLoadDocumentOptions, basepath: &str, baseuri: &str)
    -> Result<JsonLdRemoteDocument, MYERR>
{
    use std::fs::read;
    use MYERR::{
        RelativeLoadUnderBaseIRI,
        RelativeLoadRead,
        RelativeLoadNoFile,
    };
    if !url.starts_with(baseuri) {
        return Err(RelativeLoadUnderBaseIRI(baseuri.to_string()).into());
    }
    let mut rel_p: String = url.replacen(baseuri, "", 1);
    while rel_p.starts_with("/") {
        rel_p.remove(0);
    }
    let p = match get_existing_jsonld(basepath, &rel_p){
        Ok(x) => x,
        Err(msg) => return Err(RelativeLoadNoFile(msg)),
    };
    let data: Vec<u8> = match read(&p) {
        Ok(x) => x,
        Err(e) => {
            return Err(RelativeLoadRead(e.to_string()));
        },
    };
    Ok(JsonLdRemoteDocument {
        document: data,
        document_url: url.into(),
    })
}

fn loaddocument_callback_for_localfiles(
    url: &str, options: &JsonLdLoadDocumentOptions)
    -> Result<JsonLdRemoteDocument, MYERR>
{
    use std::fs::read;
    use std::path::Path;
    if !url.starts_with("file://") {
        return Err(MYERR::NotFile.into());
    }
    let data: Vec<u8> = match read(Path::new(&url.replacen("file://", "", 1))) {
        Ok(x) => x,
        Err(e) => {return Err(MYERR::LocalRead(e.to_string()).into());},
    };
    Ok(JsonLdRemoteDocument {
        document: data,
        document_url: url.into(),
    })
}

