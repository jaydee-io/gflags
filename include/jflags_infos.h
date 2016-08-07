////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_INFOS_H_
#define JFLAGS_INFOS_H_

#include "jflags_declare.h" // IWYU pragma: export

#include <vector>

namespace JFLAGS_NAMESPACE {

using std::vector;

// --------------------------------------------------------------------
// These methods are the best way to get access to info about the
// list of commandline flags.  Note that these routines are pretty slow.
//   GetAllFlags: mostly-complete info about the list, sorted by file.
//   ShowUsageWithFlags: pretty-prints the list to stdout (what --help does)
//   ShowUsageWithFlagsRestrict: limit to filenames with restrict as a substr
//
// In addition to accessing flags, you can also access argv[0] (the program
// name) and argv (the entire commandline), which we sock away a copy of.
// These variables are static, so you should only set them once.

// CommandLineFlagInfo is a client-exposed version of CommandLineFlag.
// Once it's instantiated, it has no dependencies or relationships
// with any other part of this file.
// No need to export this data only structure from DLL, avoiding VS warning
// 4251.
struct CommandLineFlagInfo
{
    std::string name;          // the name of the flag
    std::string type;          // the type of the flag: int32, etc
    std::string description;   // the "help text" associated with the flag
    std::string current_value; // the current value, as a string
    std::string default_value; // the default value, as a string
    std::string filename;      // 'cleaned' version of filename holding the flag
    bool has_validator_fn;     // true if RegisterFlagValidator called on this flag
    bool is_default;           // true if the flag has the default value and
                               // has not been set explicitly from the cmdline
                               // or via SetCommandLineOption
    const void * flag_ptr;     // pointer to the flag's current value (i.e. FLAGS_foo)
};

// Using this inside of a validator is a recipe for a deadlock.
// TODO(user) Fix locking when validators are running, to make it safe to
// call validators during ParseAllFlags.
// Also make sure then to uncomment the corresponding unit test in
// jflags_unittest.sh
extern JFLAGS_DLL_DECL void GetAllFlags(std::vector<CommandLineFlagInfo> * OUTPUT);
// These two are actually defined in jflags_reporting.cc.
extern JFLAGS_DLL_DECL void ShowUsageWithFlags(const char * argv0); // what --help does
extern JFLAGS_DLL_DECL void ShowUsageWithFlagsRestrict(const char * argv0, const char * restrict);

// Create a descriptive string for a flag.
// Goes to some trouble to make pretty line breaks.
extern JFLAGS_DLL_DECL std::string DescribeOneFlag(const CommandLineFlagInfo & flag);

// Thread-hostile; meant to be called before any threads are spawned.
extern JFLAGS_DLL_DECL void SetArgv(int argc, const char ** argv);

// The following functions are thread-safe as long as SetArgv() is
// only called before any threads start.
extern JFLAGS_DLL_DECL const std::vector<std::string> & GetArgvs();
extern JFLAGS_DLL_DECL const char * GetArgv();                    // all of argv as a string
extern JFLAGS_DLL_DECL const char * GetArgv0();                   // only argv0
extern JFLAGS_DLL_DECL uint32 GetArgvSum();                       // simple checksum of argv
extern JFLAGS_DLL_DECL const char * ProgramInvocationName();      // argv0, or "UNKNOWN" if not set
extern JFLAGS_DLL_DECL const char * ProgramInvocationShortName(); // basename(argv0)

// ProgramUsage() is thread-safe as long as SetUsageMessage() is only
// called before any threads start.
extern JFLAGS_DLL_DECL const char * ProgramUsage(); // string set by SetUsageMessage()

// VersionString() is thread-safe as long as SetVersionString() is only
// called before any threads start.
extern JFLAGS_DLL_DECL const char * VersionString(); // string set by SetVersionString()

// Set the "usage" message for this program.  For example:
//   string usage("This program does nothing.  Sample usage:\n");
//   usage += argv[0] + " <uselessarg1> <uselessarg2>";
//   SetUsageMessage(usage);
// Do not include commandline flags in the usage: we do that for you!
// Thread-hostile; meant to be called before any threads are spawned.
extern JFLAGS_DLL_DECL void SetUsageMessage(const std::string & usage);

// Sets the version string, which is emitted with --version.
// For instance: SetVersionString("1.3");
// Thread-hostile; meant to be called before any threads are spawned.
extern JFLAGS_DLL_DECL void SetVersionString(const std::string & version);

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_INFOS_H_

