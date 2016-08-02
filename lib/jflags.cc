////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
// ---
// Revamped and reorganized by Craig Silverstein
//
// This file contains the implementation of all our command line flags
// stuff.  Here's how everything fits together
//
// * FlagRegistry owns CommandLineFlags owns FlagValue.
// * FlagSaver holds a FlagRegistry (saves it at construct time,
//     restores it at destroy time).
// * CommandLineFlagParser lives outside that hierarchy, but works on
//     CommandLineFlags (modifying the FlagValues).
// * Free functions like SetCommandLineOption() work via one of the
//     above (such as CommandLineFlagParser).
//
// In more detail:
//
// -- The main classes that hold flag data:
//
// FlagValue holds the current value of a flag.  It's
// pseudo-templatized: every operation on a FlagValue is typed.  It
// also deals with storage-lifetime issues (so flag values don't go
// away in a destructor), which is why we need a whole class to hold a
// variable's value.
//
// CommandLineFlag is all the information about a single command-line
// flag.  It has a FlagValue for the flag's current value, but also
// the flag's name, type, etc.
//
// FlagRegistry is a collection of CommandLineFlags.  There's the
// global registry, which is where flags defined via DEFINE_foo()
// live.  But it's possible to define your own flag, manually, in a
// different registry you create.  (In practice, multiple registries
// are used only by FlagSaver).
//
// A given FlagValue is owned by exactly one CommandLineFlag.  A given
// CommandLineFlag is owned by exactly one FlagRegistry.  FlagRegistry
// has a lock; any operation that writes to a FlagValue or
// CommandLineFlag owned by that registry must acquire the
// FlagRegistry lock before doing so.
//
// --- Some other classes and free functions:
//
// CommandLineFlagInfo is a client-exposed version of CommandLineFlag.
// Once it's instantiated, it has no dependencies or relationships
// with any other part of this file.
//
// FlagRegisterer is the helper class used by the DEFINE_* macros to
// allow work to be done at global initialization time.
//
// CommandLineFlagParser is the class that reads from the commandline
// and instantiates flag values based on that.  It needs to poke into
// the innards of the FlagValue->CommandLineFlag->FlagRegistry class
// hierarchy to do that.  It's careful to acquire the FlagRegistry
// lock before doing any writing or other non-const actions.
//
// GetCommandLineOption is just a hook into registry routines to
// retrieve a flag based on its name.  SetCommandLineOption, on the
// other hand, hooks into CommandLineFlagParser.  Other API functions
// are, similarly, mostly hooks into the functionality described above.

#include "jflags_internals.h"

// Special flags, type 1: the 'recursive' flags.  They set another flag's val.
DECLARE_string(flagfile);
DECLARE_string(fromenv);
DECLARE_string(tryfromenv);

// Special flags, type 2: the 'parsing' flags.  They modify how we parse.
DEFINE_string(undefok, "",
              "comma-separated list of flag names that it is okay to specify "
              "on the command line even if the program does not define a flag "
              "with that name.  IMPORTANT: flags in this list that have "
              "arguments MUST use the flag=value format");

namespace JFLAGS_NAMESPACE {

using std::map;
using std::pair;
using std::sort;
using std::string;
using std::vector;

// The help message indicating that the commandline flag has been
// 'stripped'. It will not show up when doing "-help" and its
// variants. The flag is stripped if STRIP_FLAG_HELP is set to 1
// before including base/jflags.h

// This is used by this file, and also in jflags_reporting.cc
const char kStrippedFlagHelp[] = "\001\002\003\004 (unknown) \004\003\002\001";

// There are also 'reporting' flags, in jflags_reporting.cc.
static bool logging_is_probably_set_up = false;

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

// Parse a list of (comma-separated) flags.
static void ParseFlagList(const char * value, vector<string> * flags)
{
    for (const char * p = value; p && *p; value = p)
    {
        p = strchr(value, ',');
        size_t len;
        if (p)
        {
            len = p - value;
            p++;
        }
        else
        {
            len = strlen(value);
        }

        if (len == 0)
            ReportError(DIE, "ERROR: empty flaglist entry\n");
        if (value[0] == '-')
            ReportError(DIE, "ERROR: flag \"%*s\" begins with '-'\n", len, value);

        flags->push_back(string(value, len));
    }
}

// Snarf an entire file into a C++ string.  This is just so that we
// can do all the I/O in one place and not worry about it everywhere.
// Plus, it's convenient to have the whole file contents at hand.
// Adds a newline at the end of the file.
#define PFATAL(s)           \
    do                      \
    {                       \
        perror(s);          \
        jflags_exitfunc(1); \
    } while (0)

string ReadFileIntoString(const char * filename)
{
    const int kBufSize = 8092;
    char buffer[kBufSize];
    string s;
    FILE * fp;
    if ((errno = SafeFOpen(&fp, filename, "r")) != 0)
        PFATAL(filename);
    size_t n;
    while ((n = fread(buffer, 1, kBufSize, fp)) > 0)
    {
        if (ferror(fp))
            PFATAL(filename);
        s.append(buffer, n);
    }
    fclose(fp);
    return s;
}

uint32 CommandLineFlagParser::ParseNewCommandLineFlags(int * argc, char *** argv, bool remove_flags)
{
    const char * program_name = strrchr((*argv)[0], PATH_SEPARATOR); // nix path
    program_name = (program_name == NULL ? (*argv)[0] : program_name + 1);

    int first_nonopt = *argc; // for non-options moved to the end

    registry_->Lock();
    for (int i = 1; i < first_nonopt; i++)
    {
        char * arg = (*argv)[i];

        // Like getopt(), we permute non-option flags to be at the end.
        if (arg[0] != '-' ||                   // must be a program argument
            (arg[0] == '-' && arg[1] == '\0')) // "-" is an argument, not a flag
        {
            memmove((*argv) + i, (*argv) + i + 1, (*argc - (i + 1)) * sizeof((*argv)[i]));
            (*argv)[*argc - 1] = arg; // we go last
            first_nonopt--;           // we've been pushed onto the stack
            i--;                      // to undo the i++ in the loop
            continue;
        }

        if (arg[0] == '-')
            arg++; // allow leading '-'
        if (arg[0] == '-')
            arg++; // or leading '--'

        // -- alone means what it does for GNU: stop options parsing
        if (*arg == '\0')
        {
            first_nonopt = i + 1;
            break;
        }

        // Find the flag object for this option
        string key;
        const char * value;
        string error_message;
        CommandLineFlag * flag = registry_->SplitArgumentLocked(arg, &key, &value, &error_message);
        if (flag == NULL)
        {
            undefined_names_[key] = ""; // value isn't actually used
            error_flags_[key] = error_message;
            continue;
        }

        if (value == NULL)
        {
            // Boolean options are always assigned a value by SplitArgumentLocked()
            assert(strcmp(flag->type_name(), "bool") != 0);
            if (i + 1 >= first_nonopt)
            {
                // This flag needs a value, but there is nothing available
                error_flags_[key] = (string(kError) + "flag '" + (*argv)[i] + "'" + " is missing its argument");
                if (flag->help() && flag->help()[0] > '\001')
                    // Be useful in case we have a non-stripped description.
                    error_flags_[key] += string("; flag description: ") + flag->help();
                error_flags_[key] += "\n";
                break; // we treat this as an unrecoverable error
            }
            else
            {
                value = (*argv)[++i]; // read next arg for value

                // Heuristic to detect the case where someone treats a string arg
                // like a bool:
                // --my_string_var --foo=bar
                // We look for a flag of string type, whose value begins with a
                // dash, and where the flag-name and value are separated by a
                // space rather than an '='.
                // To avoid false positives, we also require the word "true"
                // or "false" in the help string.  Without this, a valid usage
                // "-lat -30.5" would trigger the warning.  The common cases we
                // want to solve talk about true and false as values.
                if (value[0] == '-' && strcmp(flag->type_name(), "string") == 0 && (strstr(flag->help(), "true") || strstr(flag->help(), "false")))
                    LOG(WARNING) << "Did you really mean to set flag '" << flag->name() << "' to the value '" << value << "'?";
            }
        }

        // TODO(csilvers): only set a flag if we hadn't set it before here
        ProcessSingleOptionLocked(flag, value, SET_FLAGS_VALUE);
    }
    registry_->Unlock();

    if (remove_flags) // Fix up argc and argv by removing command line flags
    {
        (*argv)[first_nonopt - 1] = (*argv)[0];
        (*argv) += (first_nonopt - 1);
        (*argc) -= (first_nonopt - 1);
        first_nonopt = 1; // because we still don't count argv[0]
    }

    logging_is_probably_set_up = true; // because we've parsed --logdir, etc.

    return first_nonopt;
}

string CommandLineFlagParser::ProcessFlagfileLocked(const string & flagval, FlagSettingMode set_mode)
{
    if (flagval.empty())
        return "";

    string msg;
    vector<string> filename_list;
    ParseFlagList(flagval.c_str(), &filename_list); // take a list of filenames
    for (size_t i = 0; i < filename_list.size(); ++i)
    {
        const char * file = filename_list[i].c_str();
        msg += ProcessOptionsFromStringLocked(ReadFileIntoString(file), set_mode);
    }
    return msg;
}

string CommandLineFlagParser::ProcessFromenvLocked(const string & flagval, FlagSettingMode set_mode, bool errors_are_fatal)
{
    if (flagval.empty())
        return "";

    string msg;
    vector<string> flaglist;
    ParseFlagList(flagval.c_str(), &flaglist);

    for (size_t i = 0; i < flaglist.size(); ++i)
    {
        const char * flagname = flaglist[i].c_str();
        CommandLineFlag * flag = registry_->FindFlagLocked(flagname);
        if (flag == NULL)
        {
            error_flags_[flagname] = StringPrintf("%sunknown command line flag '%s' (via --fromenv or --tryfromenv)\n", kError, flagname);
            undefined_names_[flagname] = "";
            continue;
        }

        const string envname = string("FLAGS_") + string(flagname);
        string envval;
        if (!SafeGetEnv(envname.c_str(), envval))
        {
            if (errors_are_fatal)
                error_flags_[flagname] = (string(kError) + envname + " not found in environment\n");
            continue;
        }

        // Avoid infinite recursion.
        if (envval == "fromenv" || envval == "tryfromenv")
        {
            error_flags_[flagname] = StringPrintf("%sinfinite recursion on environment flag '%s'\n", kError, envval.c_str());
            continue;
        }

        msg += ProcessSingleOptionLocked(flag, envval.c_str(), set_mode);
    }
    return msg;
}

string CommandLineFlagParser::ProcessSingleOptionLocked(CommandLineFlag * flag, const char * value, FlagSettingMode set_mode)
{
    string msg;
    if (value && !registry_->SetFlagLocked(flag, value, set_mode, &msg))
    {
        error_flags_[flag->name()] = msg;
        return "";
    }

    // The recursive flags, --flagfile and --fromenv and --tryfromenv,
    // must be dealt with as soon as they're seen.  They will emit
    // messages of their own.
    if (strcmp(flag->name(), "flagfile") == 0)
        msg += ProcessFlagfileLocked(FLAGS_flagfile, set_mode);
    else if (strcmp(flag->name(), "fromenv") == 0)
        // last arg indicates envval-not-found is fatal (unlike in --tryfromenv)
        msg += ProcessFromenvLocked(FLAGS_fromenv, set_mode, true);
    else if (strcmp(flag->name(), "tryfromenv") == 0)
        msg += ProcessFromenvLocked(FLAGS_tryfromenv, set_mode, false);

    return msg;
}

void CommandLineFlagParser::ValidateAllFlags()
{
    FlagRegistryLock frl(registry_);
    for (FlagRegistry::FlagConstIterator i = registry_->flags_.begin(); i != registry_->flags_.end(); ++i)
    {
        if (!i->second->ValidateCurrent())
        {
            // only set a message if one isn't already there.  (If there's
            // an error message, our job is done, even if it's not exactly
            // the same error.)
            if (error_flags_[i->second->name()].empty())
                error_flags_[i->second->name()] = string(kError) + "--" + i->second->name() + " must be set on the commandline (default value fails validation)\n";
        }
    }
}

bool CommandLineFlagParser::ReportErrors()
{
    // error_flags_ indicates errors we saw while parsing.
    // But we ignore undefined-names if ok'ed by --undef_ok
    if (!FLAGS_undefok.empty())
    {
        vector<string> flaglist;
        ParseFlagList(FLAGS_undefok.c_str(), &flaglist);
        for (size_t i = 0; i < flaglist.size(); ++i)
        {
            // We also deal with --no<flag>, in case the flagname was boolean
            const string no_version = string("no") + flaglist[i];
            if (undefined_names_.find(flaglist[i]) != undefined_names_.end())
                error_flags_[flaglist[i]] = ""; // clear the error message
            else if (undefined_names_.find(no_version) != undefined_names_.end())
                error_flags_[no_version] = "";
        }
    }
    // Likewise, if they decided to allow reparsing, all undefined-names
    // are ok; we just silently ignore them now, and hope that a future
    // parse will pick them up somehow.
    if (allow_command_line_reparsing)
    {
        for (map<string, string>::const_iterator it = undefined_names_.begin(); it != undefined_names_.end(); ++it)
            error_flags_[it->first] = ""; // clear the error message
    }

    bool found_error = false;
    string error_message;
    for (map<string, string>::const_iterator it = error_flags_.begin(); it != error_flags_.end(); ++it) {
        if (!it->second.empty())
        {
            error_message.append(it->second.data(), it->second.size());
            found_error = true;
        }
    }
    if (found_error)
        ReportError(DO_NOT_DIE, "%s", error_message.c_str());
    return found_error;
}

string CommandLineFlagParser::ProcessOptionsFromStringLocked(const string & contentdata, FlagSettingMode set_mode)
{
    string retval;
    const char * flagfile_contents = contentdata.c_str();
    bool flags_are_relevant = true; // set to false when filenames don't match
    bool in_filename_section = false;

    const char * line_end = flagfile_contents;
    // We read this file a line at a time.
    for (; line_end; flagfile_contents = line_end + 1)
    {
        while (*flagfile_contents && isspace(*flagfile_contents))
            ++flagfile_contents;
        // Windows uses "\r\n"
        line_end = strchr(flagfile_contents, '\r');
        if (line_end == NULL)
            line_end = strchr(flagfile_contents, '\n');

        size_t len = line_end ? line_end - flagfile_contents : strlen(flagfile_contents);
        string line(flagfile_contents, len);

        // Each line can be one of four things:
        // 1) A comment line -- we skip it
        // 2) An empty line -- we skip it
        // 3) A list of filenames -- starts a new filenames+flags section
        // 4) A --flag=value line -- apply if previous filenames match
        if (line.empty() || line[0] == '#')
        {
            // comment or empty line; just ignore
        }
        else if (line[0] == '-')
        {                                // flag
            in_filename_section = false; // instead, it was a flag-line
            if (!flags_are_relevant)     // skip this flag; applies to someone else
                continue;

            const char * name_and_val = line.c_str() + 1; // skip the leading -
            if (*name_and_val == '-')
                name_and_val++; // skip second - too
            string key;
            const char * value;
            string error_message;
            CommandLineFlag * flag = registry_->SplitArgumentLocked(
              name_and_val, &key, &value, &error_message);
            // By API, errors parsing flagfile lines are silently ignored.
            if (flag == NULL)
            {
                // "WARNING: flagname '" + key + "' not found\n"
            }
            else if (value == NULL)
            {
                // "WARNING: flagname '" + key + "' missing a value\n"
            }
            else
            {
                retval += ProcessSingleOptionLocked(flag, value, set_mode);
            }
        }
        else
        {                             // a filename!
            if (!in_filename_section) // start over: assume filenames don't match
            {
                in_filename_section = true;
                flags_are_relevant = false;
            }

            // Split the line up at spaces into glob-patterns
            const char * space = line.c_str(); // just has to be non-NULL
            for (const char * word = line.c_str(); *space; word = space + 1)
            {
                if (flags_are_relevant) // we can stop as soon as we match
                    break;
                space = strchr(word, ' ');
                if (space == NULL)
                    space = word + strlen(word);
                const string glob(word, space - word);
                // We try matching both against the full argv0 and basename(argv0)
                if (glob == ProgramInvocationName() // small optimization
                    || glob == ProgramInvocationShortName()
#if defined(HAVE_FNMATCH_H)
                    || fnmatch(glob.c_str(), ProgramInvocationName(), FNM_PATHNAME) == 0 || fnmatch(glob.c_str(), ProgramInvocationShortName(), FNM_PATHNAME) == 0
#elif defined(HAVE_SHLWAPI_H)
                    || PathMatchSpec(glob.c_str(), ProgramInvocationName()) || PathMatchSpec(glob.c_str(), ProgramInvocationShortName())
#endif
                    )
                {
                    flags_are_relevant = true;
                }
            }
        }
    }
    return retval;
}

// Now define the functions that are exported via the .h file

} // namespace JFLAGS_NAMESPACE
