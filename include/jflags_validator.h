////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_VALIDATOR_H_
#define JFLAGS_VALIDATOR_H_

#include <string>

#include "jflags_declare.h" // IWYU pragma: export

namespace JFLAGS_NAMESPACE {

// --------------------------------------------------------------------
// To actually define a flag in a file, use DEFINE_bool,
// DEFINE_string, etc. at the bottom of this file.  You may also find
// it useful to register a validator with the flag.  This ensures that
// when the flag is parsed from the commandline, or is later set via
// SetCommandLineOption, we call the validation function. It is _not_
// called when you assign the value to the flag directly using the = operator.
//
// The validation function should return true if the flag value is valid, and
// false otherwise. If the function returns false for the new setting of the
// flag, the flag will retain its current value. If it returns false for the
// default value, ParseCommandLineFlags() will die.
//
// This function is safe to call at global construct time (as in the
// example below).
//
// Example use:
//    static bool ValidatePort(const char* flagname, int32 value) {
//       if (value > 0 && value < 32768)   // value is ok
//         return true;
//       printf("Invalid value for --%s: %d\n", flagname, (int)value);
//       return false;
//    }
//    DEFINE_int32(port, 0, "What port to listen on");
//    static bool dummy = RegisterFlagValidator(&FLAGS_port, &ValidatePort);

// Returns true if successfully registered, false if not (because the
// first argument doesn't point to a command-line flag, or because a
// validator is already registered for this flag).
extern JFLAGS_DLL_DECL bool RegisterFlagValidator(const bool * flag, bool (*validate_fn)(const char *, bool));
extern JFLAGS_DLL_DECL bool RegisterFlagValidator(const int32 * flag, bool (*validate_fn)(const char *, int32));
extern JFLAGS_DLL_DECL bool RegisterFlagValidator(const uint32 * flag, bool (*validate_fn)(const char *, uint32));
extern JFLAGS_DLL_DECL bool RegisterFlagValidator(const int64 * flag, bool (*validate_fn)(const char *, int64));
extern JFLAGS_DLL_DECL bool RegisterFlagValidator(const uint64 * flag, bool (*validate_fn)(const char *, uint64));
extern JFLAGS_DLL_DECL bool RegisterFlagValidator(const double * flag, bool (*validate_fn)(const char *, double));
extern JFLAGS_DLL_DECL bool RegisterFlagValidator(const std::string * flag, bool (*validate_fn)(const char *, const std::string &));

// Convenience macro for the registration of a flag validator
#define DEFINE_validator(name, validator) static const bool name##_validator_registered = JFLAGS_NAMESPACE::RegisterFlagValidator(&FLAGS_##name, validator)

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_VALIDATOR_H_

