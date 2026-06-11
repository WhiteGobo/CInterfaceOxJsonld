use oxjsonld::{JsonLdSerializer, WriterJsonLdSerializer};

pub mod config {
    use oxjsonld::{JsonLdSerializer, WriterJsonLdSerializer};
    use oxrdf::{GraphNameRef, LiteralRef, NamedNodeRef, TermRef, NamedOrBlankNodeRef, BlankNodeRef, BlankNode};
    use std::error::Error;
    use std::collections::HashMap;

    pub struct JSONLDSerializer {
        pub config: Option<JsonLdSerializer>,
        writer: Option<WriterJsonLdSerializer<Vec<u8>>>,
        bnodemap: HashMap<String, BlankNode>,
    }

    impl JSONLDSerializer {
        pub fn new() -> Self {
            JSONLDSerializer{
                config: Some(JsonLdSerializer::new()),
                writer: None,
                bnodemap: HashMap::new(),
            }
        }

        pub fn get_bnode(&mut self, key: &str) -> Option<BlankNodeRef> {
            let m = &mut self.bnodemap;
            if m.contains_key(key){
                match m.get(key){
                    Some(bnode) => Some(bnode.as_ref()),
                    None => None,
                }
            } else {
                let bnode = BlankNode::default();
                m.insert(key.to_owned(), bnode);
                match m.get(key){
                    Some(bnode) => Some(bnode.as_ref()),
                    None => None
                }
            }
        }

        fn in_write_state(&mut self) {
            use std::mem::replace;
            match replace(&mut self.config, None) {
                None => {},
                Some(cfg) => {
                    self.writer = Some(cfg.for_writer(Vec::new()))
                },
            }
        }

        pub fn serialize_quad(&mut self, subj: NamedOrBlankNodeRef, pred: NamedNodeRef, obj: TermRef, graph: GraphNameRef) {
            use oxrdf::QuadRef;
            self.in_write_state();
            match &mut self.writer {
                Some(w) => {
                    w.serialize_quad(QuadRef::new(subj, pred, obj, graph));
                },
                None => {},
            };
        }
        

        pub fn finish(&mut self) -> Result<Vec<u8>, ()> {
            use std::mem::replace;
            self.in_write_state();
            let writer = match replace(&mut self.writer, None) {
                None => {return Err(());},
                Some(w) => w,
            };
            let mut ret = match writer.finish() {
                Ok(x) => x,
                Err(_) => {return Err(());},
            };
            ret.push(0); //ensure trailing '\0'
            Ok(ret)
        }
    }
}

pub mod genterms {
    use crate::serializer::config::{
        JSONLDSerializer,
    };
    use oxrdf::{GraphNameRef, LiteralRef, NamedNodeRef, TermRef, NamedOrBlankNodeRef};
    use std::ffi::{c_char, CStr};

    pub fn generate_IdentifiedNode<'a>(
        value: *const c_char, value_type: u8,
        serializer: &'a mut JSONLDSerializer,
        ) -> Result<NamedOrBlankNodeRef<'a>, ()>
    {
        if value.is_null(){
            return Err(());
        }
        let obj = match unsafe{ CStr::from_ptr(value) }.to_str() {
            Ok(x) => x,
            Err(_) => {return Err(());},
        };
        match value_type {
            0 => {
                match NamedNodeRef::new(obj){
                    Ok(x) => Ok(x.into()),
                    Err(_) => Err(()),
                }
            },
            1 => match serializer.get_bnode(obj) {
                Some(x) => Ok(x.into()),
                None => Err(()),
            },
            _ => Err(())
        }
    }

    pub fn generate_Term<'a>(
        value: *const c_char, value_suffix: *const c_char,
        value_type: u8,
        serializer: &'a mut JSONLDSerializer,
        ) -> Result<TermRef<'a>, ()>
    {
        if value.is_null(){
            return Err(());
        }
        let obj = match unsafe{ CStr::from_ptr(value) }.to_str() {
            Ok(x) => x,
            Err(_) => {return Err(());},
        };
        match value_type {
            0 => {
                match NamedNodeRef::new(obj){
                    Ok(x) => Ok(x.into()),
                    Err(_) => Err(()),
                }
            },
            1 => match serializer.get_bnode(obj) {
                Some(x) => Ok(x.into()),
                None => Err(()),
            },
            2 => {
                if value_suffix.is_null(){
                    Ok(LiteralRef::new_simple_literal(obj).into())
                } else {
                    let suf_c = unsafe{ CStr::from_ptr(value_suffix) };
                    let suf_iri = match suf_c.to_str() {
                        Ok(suf) => match NamedNodeRef::new(suf) {
                            Ok(x) => x,
                            Err(_) => {return Err(());},
                        },
                        Err(_) => {return Err(());},
                    };
                    Ok(LiteralRef::new_typed_literal(obj, suf_iri).into())
                }
            }
            3 => {
                if value_suffix.is_null(){
                    Ok(LiteralRef::new_language_tagged_literal_unchecked(obj, "").into())
                } else {
                    let suf_c = unsafe{ CStr::from_ptr(value_suffix) };
                    let suf = match suf_c.to_str() {
                        Ok(x) => x,
                        Err(_) => {return Err(());},
                    };
                    Ok(LiteralRef::new_language_tagged_literal_unchecked(obj, suf).into())
                }
            }
            _ => Err(())
        }
    }

    pub fn generate_IRI<'a>(value: *const c_char) -> Result<NamedNodeRef<'a>, ()>
    {
        if value.is_null() {
            return Err(());
        }
        let x: &str = match unsafe {CStr::from_ptr(value)}.to_str(){
            Ok(x) => x,
            Err(_) => {return Err(());},
        };
        match NamedNodeRef::new(x) {
            Ok(x) => Ok(x),
            Err(_) => Err(()),
        }
    }

    pub fn generate_Graph<'a>(
        graph_id: *const c_char, graph_type: u8,
        serializer: &'a mut JSONLDSerializer,
        ) -> Result<GraphNameRef<'a>, ()>
    {
        if graph_id.is_null(){
            return Ok(GraphNameRef::DefaultGraph);
        }
        let obj = match unsafe{ CStr::from_ptr(graph_id) }.to_str() {
            Ok(x) => x,
            Err(_) => {return Err(());},
        };
        match graph_type {
            0 => {
                match NamedNodeRef::new(obj){
                    Ok(x) => Ok(x.into()),
                    Err(_) => Err(()),
                }
            },
            1 => match serializer.get_bnode(obj) {
                Some(x) => Ok(x.into()),
                None => Err(()),
            },
            _ => Err(())
        }
    }
}
