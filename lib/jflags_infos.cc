////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "jflags_infos.h"
#include "FlagRegistry.h"

#include <algorithm>
#include <string>

namespace JFLAGS_NAMESPACE {

using std::string;

// --------------------------------------------------------------------
// GetAllFlags()
//    The main way the FlagRegistry class exposes its data.  This
//    returns, as strings, all the info about all the flags in
//    the main registry, sorted first by filename they are defined
//    in, and then by flagname.
// --------------------------------------------------------------------

struct FilenameFlagnameCmp
{
    bool operator()(const CommandLineFlagInfo & a, const CommandLineFlagInfo & b) const
    {
        int cmp = strcmp(a.filename.c_str(), b.filename.c_str());
        if (cmp == 0)
            cmp = strcmp(a.name.c_str(), b.name.c_str()); // secondary sort key
        return cmp < 0;
    }
};

void GetAllFlags(vector<CommandLineFlagInfo> * OUTPUT)
{
    FlagRegistry * const registry = FlagRegistry::GlobalRegistry();
    registry->Lock();
    for (FlagRegistry::FlagConstIterator i = registry->flags_.begin(); i != registry->flags_.end(); ++i)
    {
        CommandLineFlagInfo fi;
        i->second->FillCommandLineFlagInfo(&fi);
        OUTPUT->push_back(fi);
    }
    registry->Unlock();
    // Now sort the flags, first by filename they occur in, then alphabetically
    sort(OUTPUT->begin(), OUTPUT->end(), FilenameFlagnameCmp());
}

// --------------------------------------------------------------------
// SetArgv()
// GetArgvs()
// GetArgv()
// GetArgv0()
// ProgramInvocationName()
// ProgramInvocationShortName()
// SetUsageMessage()
// ProgramUsage()
//    Functions to set and get argv.  Typically the setter is called
//    by ParseCommandLineFlags.  Also can get the ProgramUsage string,
//    set by SetUsageMessage.
// --------------------------------------------------------------------

// These values are not protected by a Mutex because they are normally
// set only once during program startup.
static string argv0("UNKNOWN"); // just the program name
static string cmdline;          // the entire command-line
static string program_usage;
static vector<string> argvs;
static uint32 argv_sum = 0;

void SetArgv(int argc, const char ** argv)
{
    static bool called_set_argv = false;
    if (called_set_argv)
        return;
    called_set_argv = true;

    assert(argc > 0); // every program has at least a name
    argv0 = argv[0];

    cmdline.clear();
    for (int i = 0; i < argc; i++)
    {
        if (i != 0)
            cmdline += " ";
        cmdline += argv[i];
        argvs.push_back(argv[i]);
    }

    // Compute a simple sum of all the chars in argv
    argv_sum = 0;
    for (string::const_iterator c = cmdline.begin(); c != cmdline.end(); ++c)
        argv_sum += *c;
}

const vector<string> & GetArgvs() { return argvs; }
const char * GetArgv() { return cmdline.c_str(); }
const char * GetArgv0() { return argv0.c_str(); }
uint32 GetArgvSum() { return argv_sum; }
const char * ProgramInvocationName()
{ // like the GNU libc fn
    return GetArgv0();
}
const char * ProgramInvocationShortName()
{ // like the GNU libc fn
    size_t pos = argv0.rfind('/');
#ifdef OS_WINDOWS
    if (pos == string::npos)
        pos = argv0.rfind('\\');
#endif
    return (pos == string::npos ? argv0.c_str() : (argv0.c_str() + pos + 1));
}

void SetUsageMessage(const string & usage) { program_usage = usage; }

const char * ProgramUsage()
{
    if (program_usage.empty())
        return "Warning: SetUsageMessage() never called";
    return program_usage.c_str();
}

// --------------------------------------------------------------------
// SetVersionString()
// VersionString()
// --------------------------------------------------------------------

static string version_string;

void SetVersionString(const string & version) { version_string = version; }

const char * VersionString() { return version_string.c_str(); }


} // namespace JFLAGS_NAMESPACE

