////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------------------
// CommandLineFlagParser is the class that reads from the commandline
// and instantiates flag values based on that.  It needs to poke into
// the innards of the FlagValue->CommandLineFlag->FlagRegistry class
// hierarchy to do that.  It's careful to acquire the FlagRegistry
// lock before doing any writing or other non-const actions.
// --------------------------------------------------------------------
#ifndef JFLAGS_COMMAND_LINE_FLAG_PARSER_H_
#define JFLAGS_COMMAND_LINE_FLAG_PARSER_H_
#include "jflags_access.h"
#include "jflags_declare.h" // IWYU pragma: export

#include <string>
#include <map>

namespace JFLAGS_NAMESPACE {

class FlagRegistry;
class CommandLineFlag;

using std::string;
using std::map;

// CommandLineFlagParser
//    Parsing is done in two stages.  In the first, we go through
//    argv.  For every flag-like arg we can make sense of, we parse
//    it and set the appropriate FLAGS_* variable.  For every flag-
//    like arg we can't make sense of, we store it in a vector,
//    along with an explanation of the trouble.  In stage 2, we
//    handle the 'reporting' flags like --help and --mpm_version.
//    (This is via a call to HandleCommandLineHelpFlags(), in
//    jflags_reporting.cc.)
//    An optional stage 3 prints out the error messages.
//       This is a bit of a simplification.  For instance, --flagfile
//    is handled as soon as it's seen in stage 1, not in stage 2.
class CommandLineFlagParser
{
public:
    // The argument is the flag-registry to register the parsed flags in
    explicit CommandLineFlagParser(FlagRegistry * reg) : registry_(reg) {}
    ~CommandLineFlagParser() {}

    // Stage 1: Every time this is called, it reads all flags in argv.
    // However, it ignores all flags that have been successfully set
    // before.  Typically this is only called once, so this 'reparsing'
    // behavior isn't important.  It can be useful when trying to
    // reparse after loading a dll, though.
    uint32 ParseNewCommandLineFlags(int * argc, char *** argv, bool remove_flags);

    // Stage 2: print reporting info and exit, if requested.
    // In jflags_reporting.cc:HandleCommandLineHelpFlags().

    // Stage 3: validate all the commandline flags that have validators
    // registered.
    void ValidateAllFlags();

    // Stage 4: report any errors and return true if any were found.
    bool ReportErrors();

    // Set a particular command line option.  "newval" is a string
    // describing the new value that the option has been set to.  If
    // option_name does not specify a valid option name, or value is not
    // a valid value for option_name, newval is empty.  Does recursive
    // processing for --flagfile and --fromenv.  Returns the new value
    // if everything went ok, or empty-string if not.  (Actually, the
    // return-string could hold many flag/value pairs due to --flagfile.)
    // NB: Must have called registry_->Lock() before calling this function.
    string ProcessSingleOptionLocked(CommandLineFlag * flag, const char * value, FlagSettingMode set_mode);

    // Set a whole batch of command line options as specified by contentdata,
    // which is in flagfile format (and probably has been read from a flagfile).
    // Returns the new value if everything went ok, or empty-string if
    // not.  (Actually, the return-string could hold many flag/value
    // pairs due to --flagfile.)
    // NB: Must have called registry_->Lock() before calling this function.
    string ProcessOptionsFromStringLocked(const string & contentdata, FlagSettingMode set_mode);

    // These are the 'recursive' flags, defined at the top of this file.
    // Whenever we see these flags on the commandline, we must take action.
    // These are called by ProcessSingleOptionLocked and, similarly, return
    // new values if everything went ok, or the empty-string if not.
    string ProcessFlagfileLocked(const string & flagval, FlagSettingMode set_mode);
    // diff fromenv/tryfromenv
    string ProcessFromenvLocked(const string & flagval, FlagSettingMode set_mode, bool errors_are_fatal);

private:
    FlagRegistry * const registry_;
    map<string, string> error_flags_; // map from name to error message
    // This could be a set<string>, but we reuse the map to minimize the .o size
    map<string, string> undefined_names_; // --[flag] name was not registered
};


} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_COMMAND_LINE_FLAG_PARSER_H_

