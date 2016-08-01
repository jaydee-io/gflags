////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_ACCESS_H_
#define JFLAGS_ACCESS_H_

#include "jflags_declare.h" // IWYU pragma: export
#include "jflags_infos.h"
#include <string>

namespace JFLAGS_NAMESPACE {

// --------------------------------------------------------------------
// Normally you access commandline flags by just saying "if (FLAGS_foo)"
// or whatever, and set them by calling "FLAGS_foo = bar" (or, more
// commonly, via the DEFINE_foo macro).  But if you need a bit more
// control, we have programmatic ways to get/set the flags as well.
// These programmatic ways to access flags are thread-safe, but direct
// access is only thread-compatible.

// Return true iff the flagname was found.
// OUTPUT is set to the flag's value, or unchanged if we return false.
extern JFLAGS_DLL_DECL bool GetCommandLineOption(const char * name, std::string * OUTPUT);

// Return true iff the flagname was found. OUTPUT is set to the flag's
// CommandLineFlagInfo or unchanged if we return false.
extern JFLAGS_DLL_DECL bool GetCommandLineFlagInfo(const char * name, CommandLineFlagInfo * OUTPUT);

// Return the CommandLineFlagInfo of the flagname.  exit() if name not found.
// Example usage, to check if a flag's value is currently the default value:
//   if (GetCommandLineFlagInfoOrDie("foo").is_default) ...
extern JFLAGS_DLL_DECL CommandLineFlagInfo GetCommandLineFlagInfoOrDie(const char * name);

enum JFLAGS_DLL_DECL FlagSettingMode
{
    // update the flag's value (can call this multiple times).
    SET_FLAGS_VALUE,
    // update the flag's value, but *only if* it has not yet been updated
    // with SET_FLAGS_VALUE, SET_FLAG_IF_DEFAULT, or "FLAGS_xxx = nondef".
    SET_FLAG_IF_DEFAULT,
    // set the flag's default value to this.  If the flag has not yet updated
    // yet (via SET_FLAGS_VALUE, SET_FLAG_IF_DEFAULT, or "FLAGS_xxx = nondef")
    // change the flag's current value to the new default value as well.
    SET_FLAGS_DEFAULT
};

// Set a particular flag ("command line option").  Returns a string
// describing the new value that the option has been set to.  The
// return value API is not well-specified, so basically just depend on
// it to be empty if the setting failed for some reason -- the name is
// not a valid flag name, or the value is not a valid value -- and
// non-empty else.

// SetCommandLineOption uses set_mode == SET_FLAGS_VALUE (the common case)
extern JFLAGS_DLL_DECL std::string SetCommandLineOption(const char * name, const char * value);
extern JFLAGS_DLL_DECL std::string SetCommandLineOptionWithMode(const char * name, const char * value, FlagSettingMode set_mode);

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_ACCESS_H_

