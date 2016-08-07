////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "jflags_access.h"
#include "FlagRegistry.h"
#include "CommandLineFlagParser.h"

#include <assert.h>

namespace JFLAGS_NAMESPACE {

using std::string;

// --------------------------------------------------------------------
// GetCommandLineOption()
// GetCommandLineFlagInfo()
// GetCommandLineFlagInfoOrDie()
// SetCommandLineOption()
// SetCommandLineOptionWithMode()
//    The programmatic way to set a flag's value, using a string
//    for its name rather than the variable itself (that is,
//    SetCommandLineOption("foo", x) rather than FLAGS_foo = x).
//    There's also a bit more flexibility here due to the various
//    set-modes, but typically these are used when you only have
//    that flag's name as a string, perhaps at runtime.
//    All of these work on the default, global registry.
//       For GetCommandLineOption, return false if no such flag
//    is known, true otherwise.  We clear "value" if a suitable
//    flag is found.
// --------------------------------------------------------------------

bool GetCommandLineOption(const char * name, string * value)
{
    if (NULL == name)
        return false;
    assert(value);

    FlagRegistry * const registry = FlagRegistry::GlobalRegistry();
    FlagRegistryLock frl(registry);
    CommandLineFlag * flag = registry->FindFlagLocked(name);
    if (flag == NULL)
        return false;
    else
    {
        *value = flag->current_value();
        return true;
    }
}

bool GetCommandLineFlagInfo(const char * name, CommandLineFlagInfo * OUTPUT)
{
    if (NULL == name)
        return false;
    FlagRegistry * const registry = FlagRegistry::GlobalRegistry();
    FlagRegistryLock frl(registry);
    CommandLineFlag * flag = registry->FindFlagLocked(name);
    if (flag == NULL)
        return false;
    else
    {
        assert(OUTPUT);
        flag->FillCommandLineFlagInfo(OUTPUT);
        return true;
    }
}

CommandLineFlagInfo GetCommandLineFlagInfoOrDie(const char * name)
{
    CommandLineFlagInfo info;
    if (!GetCommandLineFlagInfo(name, &info))
    {
        fprintf(stderr, "FATAL ERROR: flag name '%s' doesn't exist\n", name);
        jflags_exitfunc(1); // almost certainly jflags_exitfunc()
    }
    return info;
}

string SetCommandLineOptionWithMode(const char * name, const char * value, FlagSettingMode set_mode)
{
    string result;
    FlagRegistry * const registry = FlagRegistry::GlobalRegistry();
    FlagRegistryLock frl(registry);
    CommandLineFlag * flag = registry->FindFlagLocked(name);
    if (flag)
    {
        CommandLineFlagParser parser(registry);
        result = parser.ProcessSingleOptionLocked(flag, value, set_mode);
        if (!result.empty())
        {
            // in the error case, we've already logged
            // Could consider logging this change
        }
    }
    // The API of this function is that we return empty string on error
    return result;
}

string SetCommandLineOption(const char * name, const char * value)
{
    return SetCommandLineOptionWithMode(name, value, SET_FLAGS_VALUE);
}

} // namespace JFLAGS_NAMESPACE

