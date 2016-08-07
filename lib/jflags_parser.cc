////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "jflags_parser.h"
#include "jflags_infos.h"
#include "jflags_declare.h" // IWYU pragma: export
#include "CommandLineFlagParser.h"
#include "FlagRegistry.h"

// Special flags, type 1: the 'recursive' flags.  They set another flag's val.
DECLARE_string(flagfile);
DECLARE_string(fromenv);
DECLARE_string(tryfromenv);

namespace JFLAGS_NAMESPACE {

// Indicates that undefined options are to be ignored.
// Enables deferred processing of flags in dynamically loaded libraries.
bool allow_command_line_reparsing = false;

// --------------------------------------------------------------------
// ParseCommandLineFlags()
// ParseCommandLineNonHelpFlags()
// HandleCommandLineHelpFlags()
//    This is the main function called from main(), to actually
//    parse the commandline.  It modifies argc and argv as described
//    at the top of jflags.h.  You can also divide this
//    function into two parts, if you want to do work between
//    the parsing of the flags and the printing of any help output.
// --------------------------------------------------------------------

static uint32 ParseCommandLineFlagsInternal(int * argc, char *** argv, bool remove_flags, bool do_report)
{
    SetArgv(*argc, const_cast<const char **>(*argv)); // save it for later

    FlagRegistry * const registry = FlagRegistry::GlobalRegistry();
    CommandLineFlagParser parser(registry);

    // When we parse the commandline flags, we'll handle --flagfile,
    // --tryfromenv, etc. as we see them (since flag-evaluation order
    // may be important).  But sometimes apps set FLAGS_tryfromenv/etc.
    // manually before calling ParseCommandLineFlags.  We want to evaluate
    // those too, as if they were the first flags on the commandline.
    registry->Lock();
    parser.ProcessFlagfileLocked(FLAGS_flagfile, SET_FLAGS_VALUE);
    // Last arg here indicates whether flag-not-found is a fatal error or not
    parser.ProcessFromenvLocked(FLAGS_fromenv, SET_FLAGS_VALUE, true);
    parser.ProcessFromenvLocked(FLAGS_tryfromenv, SET_FLAGS_VALUE, false);
    registry->Unlock();

    // Now get the flags specified on the commandline
    const int r = parser.ParseNewCommandLineFlags(argc, argv, remove_flags);

    if (do_report)
        HandleCommandLineHelpFlags(); // may cause us to exit on --help, etc.

    // See if any of the unset flags fail their validation checks
    parser.ValidateAllFlags();

    if (parser.ReportErrors()) // may cause us to exit on illegal flags
        jflags_exitfunc(1);
    return r;
}

uint32 ParseCommandLineFlags(int * argc, char *** argv, bool remove_flags)
{
    return ParseCommandLineFlagsInternal(argc, argv, remove_flags, true);
}

uint32 ParseCommandLineNonHelpFlags(int * argc, char *** argv, bool remove_flags)
{
    return ParseCommandLineFlagsInternal(argc, argv, remove_flags, false);
}

// --------------------------------------------------------------------
// AllowCommandLineReparsing()
// ReparseCommandLineNonHelpFlags()
//    This is most useful for shared libraries.  The idea is if
//    a flag is defined in a shared library that is dlopen'ed
//    sometime after main(), you can ParseCommandLineFlags before
//    the dlopen, then ReparseCommandLineNonHelpFlags() after the
//    dlopen, to get the new flags.  But you have to explicitly
//    Allow() it; otherwise, you get the normal default behavior
//    of unrecognized flags calling a fatal error.
// TODO(csilvers): this isn't used.  Just delete it?
// --------------------------------------------------------------------

void AllowCommandLineReparsing() { allow_command_line_reparsing = true; }

void ReparseCommandLineNonHelpFlags()
{
    // We make a copy of argc and argv to pass in
    const vector<string> & argvs = GetArgvs();
    int tmp_argc = static_cast<int>(argvs.size());
    char ** tmp_argv = new char *[tmp_argc + 1];
    for (int i = 0; i < tmp_argc; ++i)
        tmp_argv[i] = strdup(argvs[i].c_str()); // TODO(csilvers): don't dup

    ParseCommandLineNonHelpFlags(&tmp_argc, &tmp_argv, false);

    for (int i = 0; i < tmp_argc; ++i)
        free(tmp_argv[i]);
    delete[] tmp_argv;
}

void ShutDownCommandLineFlags() { FlagRegistry::DeleteGlobalRegistry(); }

} // namespace JFLAGS_NAMESPACE

