////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_PARSER_H_
#define JFLAGS_PARSER_H_

#include "jflags_declare.h" // IWYU pragma: export

namespace JFLAGS_NAMESPACE {

// --------------------------------------------------------------------
// The next two functions parse jflags from main():

// Looks for flags in argv and parses them.  Rearranges argv to put
// flags first, or removes them entirely if remove_flags is true.
// If a flag is defined more than once in the command line or flag
// file, the last definition is used.  Returns the index (into argv)
// of the first non-flag argument.
// See top-of-file for more details on this function.
#ifndef SWIG // In swig, use ParseCommandLineFlagsScript() instead.
extern JFLAGS_DLL_DECL uint32 ParseCommandLineFlags(int * argc, char *** argv, bool remove_flags);
#endif

// Calls to ParseCommandLineNonHelpFlags and then to
// HandleCommandLineHelpFlags can be used instead of a call to
// ParseCommandLineFlags during initialization, in order to allow for
// changing default values for some FLAGS (via
// e.g. SetCommandLineOptionWithMode calls) between the time of
// command line parsing and the time of dumping help information for
// the flags as a result of command line parsing.  If a flag is
// defined more than once in the command line or flag file, the last
// definition is used.  Returns the index (into argv) of the first
// non-flag argument.  (If remove_flags is true, will always return 1.)
extern JFLAGS_DLL_DECL uint32 ParseCommandLineNonHelpFlags(int * argc, char *** argv, bool remove_flags);

// This is actually defined in jflags_reporting.cc.
// This function is misnamed (it also handles --version, etc.), but
// it's too late to change that now. :-(
extern JFLAGS_DLL_DECL void HandleCommandLineHelpFlags(); // in jflags_reporting.cc

// Allow command line reparsing.  Disables the error normally
// generated when an unknown flag is found, since it may be found in a
// later parse.  Thread-hostile; meant to be called before any threads
// are spawned.
extern JFLAGS_DLL_DECL void AllowCommandLineReparsing();

// Reparse the flags that have not yet been recognized.  Only flags
// registered since the last parse will be recognized.  Any flag value
// must be provided as part of the argument using "=", not as a
// separate command line argument that follows the flag argument.
// Intended for handling flags from dynamically loaded libraries,
// since their flags are not registered until they are loaded.
extern JFLAGS_DLL_DECL void ReparseCommandLineNonHelpFlags();

// Clean up memory allocated by flags.  This is only needed to reduce
// the quantity of "potentially leaked" reports emitted by memory
// debugging tools such as valgrind.  It is not required for normal
// operation, or for the google perftools heap-checker.  It must only
// be called when the process is about to exit, and all threads that
// might access flags are quiescent.  Referencing flags after this is
// called will have unexpected consequences.  This is not safe to run
// when multiple threads might be running: the function is
// thread-hostile.
extern JFLAGS_DLL_DECL void ShutDownCommandLineFlags();

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_PARSER_H_

