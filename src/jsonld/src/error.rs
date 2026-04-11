use std::fmt;
use std::error;
use oxhttp::model::{StatusCode};


#[derive(Debug)]
pub enum MYERR {
    NotFile,
    LocalRead(String),
    HttpBody,
    HttpRequest(Box<dyn error::Error + Send + Sync>),
    HttpStatus(StatusCode),
    HttpBodyTransform(Box<dyn error::Error + Send + Sync>),
    RelativeLoadNoFile(String),
    RelativeLoadRead(String),
    RelativeLoadUnderBaseIRI(String),
}

impl error::Error for MYERR {}

impl fmt::Display for MYERR {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        use MYERR::{
            NotFile,
            LocalRead,
            HttpBody,
            HttpRequest,
            HttpStatus,
            HttpBodyTransform,
            RelativeLoadRead,
            RelativeLoadNoFile,
            RelativeLoadUnderBaseIRI,
        };
        match self {
            HttpBody => write!(f,
                "Failed load over http; body couldnt be constructed."),
            HttpRequest(e) => write!(f,
                "Failed load over http; request failed: {}", e),
            HttpStatus(s) => write!(f,
                "Failed load over http; unhandled response status ({}).", s),
            HttpBodyTransform(e) => write!(f,
                "Failed load over http; failed to read body, reason: {}", e),
            NotFile => write!(f,
                "Not a local file and doesnt start with 'file://'."),
            LocalRead(e) => write!(f,
                "Read error for local file, reason: {}", e),
            RelativeLoadNoFile(x) => write!(f,
                "Relative file expected under '{}' but not found.", x),
            RelativeLoadRead(e) => write!(f,
                "Read error for relative file, reason: {}", e),
            RelativeLoadUnderBaseIRI(baseiri) => write!(f,
                "Relative files expected under base IRI ({})", baseiri),
        }
    }
}


#[derive(Debug)]
pub enum MyIoErr {
    NotFound(Vec<String>),
    Fatal,
}

impl error::Error for MyIoErr {}

impl fmt::Display for MyIoErr {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        use MyIoErr::{NotFound, Fatal};
        match self {
            NotFound(errors) => {
                let mut ret = match
                    write!(f, "Couldnt load document from IRI. Reasons:\n")
                {
                    Err(e) => {return Err(e);},
                    x => x,
                };
                for msg in errors {
                    ret = match write!(f, "{}\n", msg){
                        Err(e) => {return Err(e);},
                        x => x,
                    };
                }
                ret
            },
            Fatal => {
                write!(f, "Fatal Error during documentloader callback")
            }
        }
    }
}
