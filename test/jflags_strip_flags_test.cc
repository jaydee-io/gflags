////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
// ---
// Author: csilvers@google.com (Craig Silverstein)
//
// A simple program that uses STRIP_FLAG_HELP.  We'll have a shell
// script that runs 'strings' over this program and makes sure
// that the help string is not in there.

#define STRIP_FLAG_HELP 1
#include <jflags/jflags.h>

#include <stdio.h>

using JFLAGS_NAMESPACE::SetUsageMessage;
using JFLAGS_NAMESPACE::ParseCommandLineFlags;


DEFINE_bool(test, true, "This text should be stripped out");

int main(int argc, char** argv) {
  SetUsageMessage("Usage message");
  ParseCommandLineFlags(&argc, &argv, false);

  // Unfortunately, for us, libtool can replace executables with a shell
  // script that does some work before calling the 'real' executable
  // under a different name.  We need the 'real' executable name to run
  // 'strings' on it, so we construct this binary to print the real
  // name (argv[0]) on stdout when run.
  puts(argv[0]);

  return 0;
}
