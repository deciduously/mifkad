@0xd2f4406e1b4d718a;

struct Kid {
  name @0 :Text;
  schedule @1 :List(Day);

  struct Day {
    name @0 :Text;

    union {
      core @1 :Text;
      extended @2 :Text;
      unscheduled @3 :Void;
    }
  }
}

struct Classroom {
  letter @0 :Text;
  capacity @1 :UInt8;
  kids @2 :List(Kid);
}
