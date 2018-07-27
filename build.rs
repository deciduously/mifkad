extern crate capnpc;

fn main() {
    capnpc::CompilerCommand::new()
        .src_prefix("schema")
        .file("schema/enrollment.capnp")
        .run().expect("schema compiler command");
}
