////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_INTERNALS_H_ 
#define JFLAGS_INTERNALS_H_

#include "jflags.h"
#include "jflags_error.h"
#include "config.h"

#include <ctype.h>
#if defined(HAVE_FNMATCH_H)
#include <fnmatch.h>
#elif defined(HAVE_SHLWAPI_H)
#include <shlwapi.h>
#endif
#include <string.h>

#include <algorithm>
#include <map>
#include <vector>

#include "FlagValue.h"
#include "CommandLineFlag.h"
#include "FlagRegistry.h"

namespace JFLAGS_NAMESPACE {

using std::map;
using std::sort;
using std::vector;

string ReadFileIntoString(const char * filename);

extern bool allow_command_line_reparsing;

// --------------------------------------------------------------------
// FlagSaverImpl
//    This class stores the states of all flags at construct time,
//    and restores all flags to that state at destruct time.
//    Its major implementation challenge is that it never modifies
//    pointers in the 'main' registry, so global FLAG_* vars always
//    point to the right place.
// --------------------------------------------------------------------

class FlagSaverImpl
{
public:
    // Constructs an empty FlagSaverImpl object.
    explicit FlagSaverImpl(FlagRegistry * main_registry);
    ~FlagSaverImpl();

    // Saves the flag states from the flag registry into this object.
    // It's an error to call this more than once.
    // Must be called when the registry mutex is not held.
    void SaveFromRegistry();

    // Restores the saved flag states into the flag registry.  We
    // assume no flags were added or deleted from the registry since
    // the SaveFromRegistry; if they were, that's trouble!  Must be
    // called when the registry mutex is not held.
    void RestoreToRegistry();

private:
    FlagRegistry * const main_registry_;
    vector<CommandLineFlag *> backup_registry_;

    FlagSaverImpl(const FlagSaverImpl &); // no copying!
    void operator=(const FlagSaverImpl &);
};

// --------------------------------------------------------------------
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
// --------------------------------------------------------------------

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

#endif // JFLAGS_INTERNALS_H_
