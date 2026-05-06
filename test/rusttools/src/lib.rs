//!Compare two rdf datasets
//!
//!Use default graph as blank node
//!When generating the graph, all equal terms which are present in a rdf dataset
//!as subject, object or as graph, are mapped to a single node(termnode).
//!Also each edge is mapped to a node(edgenode) ignoring any shared predicates.
//!To represent a quad, the edgenodes are connected
//!with weighted edges(subj, obj, graph) to the nodes representing their
//!subject, predicate and graphs:
//!\dot
//!digraph G {
//!    graph[compound=true];
//!    quad[shape=record, label="{quad|{ |subject|predicate|object|graph}}"];
//!    subgraph cluster{
//!        edge[arrowhead=none];
//!        edge_node[label="edgenode\n(0, predicate)"];
//!        edge_node -> subject;
//!        edge_node -> object;
//!        edge_node -> graph_id;
//!    }
//!    quad -> edge_node[lhead=cluster];
//!}
//!\enddot
//!
use std::fmt;
use std::cmp::Ordering;
use petgraph::Undirected;
use petgraph::graphmap::GraphMap;
use std::os::raw::c_char;
use std::ffi::CStr;


pub enum GraphType {
    Default_,
    Anonym(String),
    Identified(String),
}

#[derive(PartialEq, Debug)]
enum DSEdge {
    Subject,
    Object,
    Graph,
}

impl fmt::Display for DSEdge {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        use DSEdge::{Subject, Object, Graph};
        match self {
            Subject => write!(f, "subj"),
            Object => write!(f, "obj"),
            Graph => write!(f, "graph"),
        }
    }
}


#[derive(Copy, Clone, Hash)]
enum DSNode<'a> {
    IRI(&'a str),
    BNode(Option<&'a str>),
    TypedLiteral(&'a str, &'a str),
    LangLiteral(&'a str, &'a str),
    Edge(usize, &'a str),
}
impl DSNode<'_> {
    fn can_be_mapped_to(&self, other: &Self) -> bool {
        use DSNode::{IRI, BNode, TypedLiteral, LangLiteral, Edge};
        match (self, other){
            (BNode(_), BNode(_)) => true,
            (Edge(_, iri1), Edge(_, iri2)) => iri1 == iri2,
            (IRI(iri1), IRI(iri2)) => iri1 == iri2,
            (TypedLiteral(val1, suf1), TypedLiteral(val2, suf2))
                => (val1 == val2) && (suf1 == suf2),
            (LangLiteral(val1, lang1), LangLiteral(val2, lang2))
                => (val1 == val2) && (lang1 == lang2),
            _ => false,
        }
    }
}
impl fmt::Display for DSNode<'_> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        use DSNode::{IRI, BNode, TypedLiteral, LangLiteral, Edge};
        match self {
            IRI(x) => write!(f, "<{}>", x),
            BNode(x) => write!(f, "_:{:?}", x),
            TypedLiteral(x, y) => write!(f, "\"{}\"^^{}", x, y),
            LangLiteral(x, y) => write!(f, "\"{}\"@{}", x, y),
            Edge(x, y) => write!(f, "({}, {})", x, y),
        }
    }
}


impl Ord for DSNode<'_> {
    fn cmp(&self, other: &Self) -> Ordering {
        match (self, other) {
            (DSNode::IRI(x), DSNode::IRI(y)) => {return x.cmp(y);},
            (DSNode::BNode(x), DSNode::BNode(y)) => {return x.cmp(y);},
            (DSNode::TypedLiteral(x1, x2), DSNode::TypedLiteral(y1, y2)) => {
                match x1.cmp(y1){
                    Ordering::Equal => {},
                    x => {return x;},
                }
                return x2.cmp(y2);
            },
            (DSNode::LangLiteral(x1, x2), DSNode::LangLiteral(y1, y2)) => {
                match x1.cmp(y1){
                    Ordering::Equal => {},
                    x => {return x;},
                }
                return x2.cmp(y2);
            },
            (DSNode::Edge(_, x2), DSNode::Edge(_, y2)) => {
                return x2.cmp(y2);
            },
            (DSNode::IRI(_), _) => {return Ordering::Less;},
            (_, DSNode::IRI(_)) => {return Ordering::Greater;},
            (DSNode::BNode(_), _) => {return Ordering::Less;},
            (_, DSNode::BNode(_)) => {return Ordering::Greater;},
            (DSNode::TypedLiteral(..), _) => {return Ordering::Less;},
            (_, DSNode::TypedLiteral(..)) => {return Ordering::Greater;},
            (DSNode::LangLiteral(..), _) => {return Ordering::Less;},
            (_, DSNode::LangLiteral(..)) => {return Ordering::Greater;},
        }
    }
}

impl PartialOrd for DSNode<'_> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl PartialEq for DSNode<'_> {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (DSNode::IRI(x), DSNode::IRI(y)) => {return x == y;},
            (DSNode::BNode(x), DSNode::BNode(y)) => {return x == y;},
            (DSNode::TypedLiteral(x1, x2), DSNode::TypedLiteral(y1, y2)) => {
                if x1 != y1 {return false;}
                return x2 == y2;
            },
            (DSNode::LangLiteral(x1, x2), DSNode::LangLiteral(y1, y2)) => {
                if x1 != y1 {return false;}
                return x2 == y2;
            },
            (DSNode::Edge(i1, p1), DSNode::Edge(i2, p2)) => {
                (p1 == p2) && (i1==i2)
            },
            _ => false
        }
    }
}

impl Eq for DSNode<'_> {}


pub struct TripleStream {
    pub triples: Vec<(String, u8, String, String, Option<String>, u8, GraphType)>,
}


impl TripleStream {
    fn new() -> Self {
        TripleStream {
            triples: Vec::new(),
        }
    }


    pub fn add_edge(&mut self, 
        subject_value: &str, subject_type: u8,
        predicate_value: &str,
        object_value: &str, object_suffix: Option<&str>, object_type: u8,
        graph: Option<&str>, graph_type: u8,
        ) -> Result<(), ()>
    {
        let subj_v: String = subject_value.to_string();
        let pred_v: String = predicate_value.to_string();
        let obj_v: String = object_value.to_string();
        let obj_s: Option<String> = match object_suffix{
            Some(x) => Some(x.to_string()),
            None => None,
        };
        let graph_id = match graph {
            None => GraphType::Default_,
            Some(x) => match graph_type {
                0 => GraphType::Anonym(x.to_string()),
                1 => GraphType::Identified(x.to_string()),
                _ => {return Err(());}
            },
        };
        self.triples.push((subj_v, subject_type, pred_v, obj_v, obj_s, object_type, graph_id));
        Ok(())
    }
}



#[unsafe(no_mangle)]
pub extern "C" fn new_TripleStream<'a>() -> *mut TripleStream {
    let instance = TripleStream::new();
    let mybox = Box::new(instance);
    Box::into_raw(mybox)
}

#[unsafe(no_mangle)]
pub extern "C" fn free_TripleStream(stream: *mut TripleStream){
    unsafe { let _ = Box::from_raw(stream); }
}

#[unsafe(no_mangle)]
pub extern "C" fn append_TripleStream(
    stream: *mut TripleStream,
    subject_value: *const c_char, subject_type: u8,
    predicate_value: *const c_char,
    object_value: *const c_char, object_suffix: *const c_char,
    object_type: u8,
    graph: *const c_char, graph_type: u8)
{
    let subj_w: &str = match unsafe {CStr::from_ptr(subject_value)}.to_str(){
        Ok(x) => x,
        Err(_) => {return;},
    };
    let pred_w: &str = match unsafe {CStr::from_ptr(predicate_value)}.to_str(){
        Ok(x) => x,
        Err(_) => {return;},
    };
    let obj_w: &str = match unsafe {CStr::from_ptr(object_value)}.to_str(){
        Ok(x) => x,
        Err(_) => {return;},
    };
    let obj_s = if object_suffix.is_null() {
        None
    } else {
        match unsafe{CStr::from_ptr(object_suffix).to_str()} {
            Ok(x) => Some(x),
            Err(_) => {return;},
        }
    };
    let graph_s = if graph.is_null(){
        None
    } else {
        match unsafe{CStr::from_ptr(graph).to_str()} {
            Ok(x) => Some(x),
            Err(_) => {return;},
        }
    };
    match unsafe {(*stream)
        .add_edge(subj_w, subject_type, pred_w, obj_w, obj_s, object_type, graph_s, graph_type)
    } {
        Ok(()) => {},
        Err(()) => {},
    };
}


fn compare_edgeweight<'a, 'b>(x: &DSEdge, y: &DSEdge) -> bool {
    x == y
}

fn compare_nodeweight<'a, 'b>(x: &'a DSNode, y: &'b DSNode) -> bool {
    x.can_be_mapped_to(y)
}


fn my_create_graph<'a>(
    edges: &'a Vec<(String, u8, String, String, Option<String>, u8, GraphType)>)
    -> Result<GraphMap<DSNode<'a>, DSEdge, Undirected>, ()>
{
    let default_graph_id = DSNode::BNode(None);
    let mut ret = GraphMap::new();
    let mut i: usize = 0;
    for (subj_v, subj_type, pred_v, obj_v, obj_s, obj_type, graph_id) in edges {
        let x = match subj_type {
            0 => DSNode::IRI(&subj_v),
            1 => DSNode::BNode(Some(&subj_v)),
            _ => {return Err(());}
        };
        let y = match obj_type {
            0 => DSNode::IRI(&obj_v),
            1 => DSNode::BNode(Some(&obj_v)),
            2 => {
                let dt = match obj_s {
                    Some(x) => x,
                    None => "http://www.w3.org/2001/XMLSchema#string",
                };
                DSNode::TypedLiteral(&obj_v, dt)
            },
            3 => {
                let lang = match obj_s {
                    Some(x) => x,
                    None => "",
                };
                DSNode::LangLiteral(&obj_v, lang)
            },
            _ => {return Err(());}
        };
        let e = DSNode::Edge(i, pred_v);
        i += 1;
        let g_id = match graph_id {
            GraphType::Default_ => default_graph_id,
            GraphType::Anonym(x) => DSNode::BNode(Some(x)),
            GraphType::Identified(x) => DSNode::IRI(x),
        };
        let q1 = ret.add_edge(x, e, DSEdge::Subject);
        let q2 = ret.add_edge(e, y, DSEdge::Object);
        let q3 = ret.add_edge(e, g_id, DSEdge::Graph);
    }
    Ok(ret)
}



#[unsafe(no_mangle)]
pub extern "C" fn compare_triples(
    first: *mut TripleStream, second: *mut TripleStream)
    -> bool
{
use petgraph::algo::is_isomorphic_matching;
use petgraph::algo::is_isomorphic;
    eprintln!("create graph 1");
    let g1 = match my_create_graph(unsafe{&(*first).triples}){
        Ok(x) => x,
        Err(_) => {return false;},
    };
    eprintln!("create graph 2");
    let g2 = match my_create_graph(unsafe{&(*second).triples}) {
        Ok(x) => x,
        Err(_) => {return false;},
    };
    eprintln!("compare graphs:");
    eprintln!("{} {} {} {}", g1.node_count(), g2.node_count(), g1.edge_count(), g2.edge_count());
    let ret = is_isomorphic_matching(
        &g1, &g2,
        compare_nodeweight, compare_edgeweight);
    eprintln!("compare graphs complete {}:", ret);
    for (n1, n2, e) in g1.all_edges() {
        eprintln!("qwertz1 {} {} {}", n1, n2, e);
    }
    for (n1, n2, e) in g2.all_edges() {
        eprintln!("qwertz2 {} {} {}", n1, n2, e);
    }
    ret
}
