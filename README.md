jflags is a C++ library that implements commandline flags processing.
It's a fork of gflags library, with the following improvements :
 - Compatible with the gflags API (Well, at least for the most used ones ;-))
 - Unix-like flag naming convention (e.g. `mySuperFlag`flag is parsed from command line as `--my-super-flag`)
 - New API to define a short name for a flag (e.g. `DEFINE_short_bool(mySuperFlag, 'm', ...)` to use `-m` on command line)
 - _User friendly_ instead of gflags' _developper friendly_ help message (no source file name, no type information, ...)
 - New API to declare a flag with a category
 - Oh... and also moved to C++11
