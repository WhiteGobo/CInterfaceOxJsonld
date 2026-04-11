use std::ptr;
use std::fmt;
use std::error;
use std::ffi::CString;
use std::os::raw::{c_char, c_void};
use oxrdf::Quad;


enum TERMTYPE {
    Uri = 0,
    BNode = 1,
    TypedLiteral = 2,
    LangLiteral = 3,
}

pub type TripleHandler = extern "C" fn(
    *const c_char, u8,
    *const c_char,
    *const c_char, *const c_char, u8,
    *const c_char, u8,
    *mut c_void) -> i8;



fn create_optional_ptr(x: Option<String>)
    -> Result<(CString, *const c_char), String>
{
    // destroys memory if x_c is Option<CString>
    let (x_c, tmp) = match x.clone() {
        Some(x) => (match CString::new(x) {
            Ok(y) => y,
            _=>{return Err("".to_string());}
        }, true),
        None => (CString::new("").unwrap(), false),
    };
    let x_ptr = match tmp {
        true => x_c.as_ptr(),
        false => ptr::null(),
    };
    return Ok((x_c, x_ptr));
}

#[derive(Debug)]
pub enum CallHookError {
    DuringCall(i8),
    TransformSubjVal,
    TransformPredVal,
    TransformObjVal,
    TransformObjSuffix,
    TransformGraphId,
}
impl error::Error for CallHookError {}

impl fmt::Display for CallHookError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        use CallHookError::{DuringCall, TransformSubjVal, TransformPredVal,
                TransformObjVal, TransformObjSuffix,TransformGraphId};
        match self {
            DuringCall(x) => write!(f, "call_triplehandler failed with {}", x),
            TransformSubjVal => write!(f, "failed subject"),
            TransformPredVal => write!(f, "failed predicate"),
            TransformObjVal => write!(f, "failed object value"),
            TransformObjSuffix => write!(f, "failed object suffix"),
            TransformGraphId => write!(f, "failed graph id"),
        }
    }
}



pub fn call_hook(quad: Quad, hook:TripleHandler, hook_data: *mut c_void)
    -> Result<(), CallHookError>
{
    use oxrdf::{NamedOrBlankNode, Term, GraphName};
    let (subj_type, subj_val) = match quad.subject {
        NamedOrBlankNode::NamedNode(x) => (TERMTYPE::Uri, x.into_string()),
        NamedOrBlankNode::BlankNode(x) => (TERMTYPE::BNode, x.into_string()),
    };
    let pred_val = quad.predicate.into_string();
    let (obj_type, obj_val, obj_suffix): (TERMTYPE, String, Option<String>) = match quad.object {
        Term::NamedNode(x) => (TERMTYPE::Uri, x.into_string(), None),
        Term::BlankNode(x) => (TERMTYPE::BNode, x.into_string(), None),
        Term::Literal(x) => match (x.language(), x.datatype().as_str()) {
            (Some(l), _) => (
                TERMTYPE::LangLiteral, x.value().to_string(),
                Some(l.to_string())),
            (_, "http://www.w3.org/2001/XMLSchema#string")
                => (TERMTYPE::TypedLiteral, x.value().to_string(), None),
            (_, d)
                => (TERMTYPE::TypedLiteral, x.value().to_string(), Some(d.to_string())),
        },
    };
    let (graph_type, graph_val) = match quad.graph_name {
        GraphName::NamedNode(x) => (TERMTYPE::Uri, Some(x.into_string())),
        GraphName::BlankNode(x) => (TERMTYPE::BNode, Some(x.into_string())),
        GraphName::DefaultGraph => (TERMTYPE::Uri, None),
    };
    let Ok(subj_val_c) = CString::new(subj_val) else {
        return Err(CallHookError::TransformSubjVal);
    };
    let Ok(pred_val_c) = CString::new(pred_val) else {
        return Err(CallHookError::TransformPredVal);
    };
    let Ok(obj_val_c) = CString::new(obj_val) else {
        return Err(CallHookError::TransformObjVal);
    };
    let (_obj_suffix_c, obj_suffix_ptr) = match create_optional_ptr(obj_suffix){
        Ok(x) => x,
        _ => {return Err(CallHookError::TransformObjSuffix);}
    };
    let (_graph_id_c, graph_id_ptr) = match create_optional_ptr(graph_val){
        Ok(x) => x,
        _ => {return Err(CallHookError::TransformGraphId);}
    };
    let err = hook(
        subj_val_c.as_ptr(), subj_type as u8,
        pred_val_c.as_ptr(),
        obj_val_c.as_ptr(), obj_suffix_ptr, obj_type as u8,
        graph_id_ptr, graph_type as u8,
        hook_data);
    if err != 0 {
        return Err(CallHookError::DuringCall(err))
    }
    return Ok(());
}

