use oxjsonld::{JsonLdSerializer, WriterJsonLdSerializer};

pub mod config {
    use oxjsonld::{JsonLdSerializer, WriterJsonLdSerializer};
    use oxrdf::{GraphNameRef, LiteralRef, NamedNodeRef, TermRef, NamedOrBlankNodeRef};
    use std::error::Error;

    pub struct JSONLDSerializer {
        pub config: Option<JsonLdSerializer>,
        writer: Option<WriterJsonLdSerializer<Vec<u8>>>,
    }

    impl JSONLDSerializer {
        pub fn new() -> Self {
            JSONLDSerializer{
                config: Some(JsonLdSerializer::new()),
                writer: None,
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
            let writer = match replace(&mut self.writer, None) {
                None => {return Err(());},
                Some(w) => w,
            };
            let ret = match writer.finish() {
                Ok(x) => x,
                Err(_) => {return Err(());},
            };
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
        subject: *const c_char, subject_type: u8,
        serializer: *mut JSONLDSerializer,
        ) -> Result<NamedOrBlankNodeRef<'a>, ()>
    {
        match NamedNodeRef::new("http://example.com/node") {
            Ok(x) => Ok(x.into()),
            Err(_) => Err(()),
        }
    }

    pub fn generate_Term<'a>(
        object: *const c_char, object_suffix: *const c_char,
        object_type: u8,
        serializer: *mut JSONLDSerializer,
        ) -> Result<TermRef<'a>, ()>
    {
        match NamedNodeRef::new("http://example.com/node"){
            Ok(x) => Ok(x.into()),
            Err(_) => Err(()),
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
        serializer: *mut JSONLDSerializer,
        ) -> Result<GraphNameRef<'a>, ()>
    {
        Ok(GraphNameRef::DefaultGraph)
    }
}
