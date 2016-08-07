////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "jflags_deprecated.h"
#include "jflags_internals.h"
#include "FlagSaver.h"
#include <string>

namespace JFLAGS_NAMESPACE {

using std::string;

// --------------------------------------------------------------------
// CommandlineFlagsIntoString()
// ReadFlagsFromString()
// AppendFlagsIntoFile()
// ReadFromFlagsFile()
//    These are mostly-deprecated routines that stick the
//    commandline flags into a file/string and read them back
//    out again.  I can see a use for CommandlineFlagsIntoString,
//    for creating a flagfile, but the rest don't seem that useful
//    -- some, I think, are a poor-man's attempt at FlagSaver --
//    and are included only until we can delete them from callers.
//    Note they don't save --flagfile flags (though they do save
//    the result of having called the flagfile, of course).
// --------------------------------------------------------------------

static string TheseCommandlineFlagsIntoString(const vector<CommandLineFlagInfo> & flags)
{
    vector<CommandLineFlagInfo>::const_iterator i;

    size_t retval_space = 0;
    for (i = flags.begin(); i != flags.end(); ++i)
        // An (over)estimate of how much space it will take to print this flag
        retval_space += i->name.length() + i->current_value.length() + 5;

    string retval;
    retval.reserve(retval_space);
    for (i = flags.begin(); i != flags.end(); ++i)
    {
        retval += "--";
        retval += i->name;
        retval += "=";
        retval += i->current_value;
        retval += "\n";
    }
    return retval;
}

string CommandlineFlagsIntoString()
{
    vector<CommandLineFlagInfo> sorted_flags;
    GetAllFlags(&sorted_flags);
    return TheseCommandlineFlagsIntoString(sorted_flags);
}

bool ReadFlagsFromString(const string & flagfilecontents,
                         const char * /*prog_name*/, // TODO(csilvers): nix this
                         bool errors_are_fatal)
{
    FlagRegistry * const registry = FlagRegistry::GlobalRegistry();
    FlagSaver saved_states;

    CommandLineFlagParser parser(registry);
    registry->Lock();
    parser.ProcessOptionsFromStringLocked(flagfilecontents, SET_FLAGS_VALUE);
    registry->Unlock();
    // Should we handle --help and such when reading flags from a string?  Sure.
    HandleCommandLineHelpFlags();
    if (parser.ReportErrors())
    {
        // Error.  Restore all global flags to their previous values.
        if (errors_are_fatal)
            jflags_exitfunc(1);
        return false;
    }
    saved_states.discard();
    return true;
}

// TODO(csilvers): nix prog_name in favor of ProgramInvocationShortName()
bool AppendFlagsIntoFile(const string & filename, const char * prog_name)
{
    FILE * fp;
    if (SafeFOpen(&fp, filename.c_str(), "a") != 0)
        return false;

    if (prog_name)
        fprintf(fp, "%s\n", prog_name);

    vector<CommandLineFlagInfo> flags;
    GetAllFlags(&flags);
    // But we don't want --flagfile, which leads to weird recursion issues
    vector<CommandLineFlagInfo>::iterator i;
    for (i = flags.begin(); i != flags.end(); ++i)
    {
        if (strcmp(i->name.c_str(), "flagfile") == 0)
        {
            flags.erase(i);
            break;
        }
    }
    fprintf(fp, "%s", TheseCommandlineFlagsIntoString(flags).c_str());

    fclose(fp);
    return true;
}

bool ReadFromFlagsFile(const string & filename, const char * prog_name, bool errors_are_fatal)
{
    return ReadFlagsFromString(ReadFileIntoString(filename.c_str()), prog_name, errors_are_fatal);
}

} // namespace JFLAGS_NAMESPACE

