// enrollment.rs deals with the capnproto stuff
use capnp::serialize_packed;
use enrollment_capnp::{classroom, kid, school};
use errors::*;

// BEN - this might be a waste of effort.  Let's get 'em into Rust structs for now

pub fn write_classroom() -> ::std::io::Result<()> {
    let mut message = ::capnp::message::Builder::new_default();
    {
        let school = message.init_root::<school::Builder>();
        let mut classrooms = school.init_classrooms(1);
        {
            let mut class_a = classrooms.reborrow().get(0);
            class_a.set_letter("A");
            class_a.set_capacity(15);
            let mut kids = class_a.init_kids(2); // Start with two kids - this is going to change
                                                 // Set up Ben
            {
                let mut ben = kids.reborrow().get(0);
                ben.set_id(123);
                ben.set_name("Ben Lovy");
                // SHOULD BE 5
                {
                    let mut schedule = ben.reborrow().init_schedule(1);
                    schedule.reborrow().get(0).set_name("Monday");
                    schedule.reborrow().get(0).get_expected().set_core("8-4");
                }
            }

            // Set up Marisa
            {
                let mut marisa = kids.reborrow().get(1);
                marisa.set_id(124);
                marisa.set_name("Marisa Sileo");
                // SHOULD BE 5
                {
                    let mut schedule = marisa.reborrow().init_schedule(1);
                    schedule.reborrow().get(0).set_name("Monday");
                    schedule
                        .reborrow()
                        .get(0)
                        .get_expected()
                        .set_unscheduled(());
                }
            }
        }
    }
    // write Ben and Marisa
    serialize_packed::write_message(&mut ::std::io::stdout(), &message)
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test_write_classroom() {
        write_classroom().unwrap();
        assert_eq!("write", "write")
    }
}
