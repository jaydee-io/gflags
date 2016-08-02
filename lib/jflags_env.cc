////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "jflags_env.h"
#include "jflags_internals.h"

namespace JFLAGS_NAMESPACE {

// --------------------------------------------------------------------
// GetFromEnv()
//    These is helper function for routines like BoolFromEnv()
//    defined below.  They're defined here so they can live in the 
//    unnamed namespace (which makes friendship declarations for these
//    classes possible).
// --------------------------------------------------------------------

template <typename T>
T GetFromEnv(const char * varname, const char * type, T dflt)
{
    std::string valstr;
    if (SafeGetEnv(varname, valstr)) {
        FlagValue ifv(new T, type, true);
        if (!ifv.ParseFrom(valstr.c_str()))
            ReportError(DIE, "ERROR: error parsing env variable '%s' with value '%s'\n", varname, valstr.c_str());
        return OTHER_VALUE_AS(ifv, T);
    }
    else
        return dflt;
}

// --------------------------------------------------------------------
// BoolFromEnv()
// Int32FromEnv()
// Uint32FromEnv()
// Int64FromEnv()
// Uint64FromEnv()
// DoubleFromEnv()
// StringFromEnv()
//    Reads the value from the environment and returns it.
//    We use an FlagValue to make the parsing easy.
//    Example usage:
//       DEFINE_bool(myflag, BoolFromEnv("MYFLAG_DEFAULT", false), "whatever");
// --------------------------------------------------------------------

bool BoolFromEnv(const char * v, bool dflt)
{
    return GetFromEnv(v, "bool", dflt);
}
int32 Int32FromEnv(const char * v, int32 dflt)
{
    return GetFromEnv(v, "int32", dflt);
}
uint32 Uint32FromEnv(const char * v, uint32 dflt)
{
    return GetFromEnv(v, "uint32", dflt);
}
int64 Int64FromEnv(const char * v, int64 dflt)
{
    return GetFromEnv(v, "int64", dflt);
}
uint64 Uint64FromEnv(const char * v, uint64 dflt)
{
    return GetFromEnv(v, "uint64", dflt);
}
double DoubleFromEnv(const char * v, double dflt)
{
    return GetFromEnv(v, "double", dflt);
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // ignore getenv security warning
#endif
const char * StringFromEnv(const char * varname, const char * dflt)
{
    const char * const val = getenv(varname);
    return val ? val : dflt;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace JFLAGS_NAMESPACE

