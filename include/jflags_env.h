////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_ENV_H_
#define JFLAGS_ENV_H_

#include "jflags_declare.h" // IWYU pragma: export

namespace JFLAGS_NAMESPACE {

// --------------------------------------------------------------------
// Useful routines for initializing flags from the environment.
// In each case, if 'varname' does not exist in the environment
// return defval.  If 'varname' does exist but is not valid
// (e.g., not a number for an int32 flag), abort with an error.
// Otherwise, return the value.  NOTE: for booleans, for true use
// 't' or 'T' or 'true' or '1', for false 'f' or 'F' or 'false' or '0'.

extern JFLAGS_DLL_DECL bool BoolFromEnv(const char * varname, bool defval);
extern JFLAGS_DLL_DECL int32 Int32FromEnv(const char * varname, int32 defval);
extern JFLAGS_DLL_DECL uint32 Uint32FromEnv(const char * varname, uint32 defval);
extern JFLAGS_DLL_DECL int64 Int64FromEnv(const char * varname, int64 defval);
extern JFLAGS_DLL_DECL uint64 Uint64FromEnv(const char * varname, uint64 defval);
extern JFLAGS_DLL_DECL double DoubleFromEnv(const char * varname, double defval);
extern JFLAGS_DLL_DECL const char * StringFromEnv(const char * varname, const char * defval);


} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_ENV_H_

