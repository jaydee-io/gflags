////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "jflags_validator.h"
#include "jflags_internals.h"

namespace JFLAGS_NAMESPACE {

// --------------------------------------------------------------------
// AddFlagValidator()
//    These are helper functions for routines like BoolFromEnv() and
//    RegisterFlagValidator, defined below.  They're defined here so
//    they can live in the unnamed namespace (which makes friendship
//    declarations for these classes possible).
// --------------------------------------------------------------------

bool AddFlagValidator(const void * flag_ptr, ValidateFnProto validate_fn_proto)
{
    // We want a lock around this routine, in case two threads try to
    // add a validator (hopefully the same one!) at once.  We could use
    // our own thread, but we need to loook at the registry anyway, so
    // we just steal that one.
    FlagRegistry * const registry = FlagRegistry::GlobalRegistry();
    FlagRegistryLock frl(registry);
    // First, find the flag whose current-flag storage is 'flag'.
    // This is the CommandLineFlag whose current_->value_buffer_ == flag
    CommandLineFlag * flag = registry->FindFlagViaPtrLocked(flag_ptr);
    if (!flag)
    {
        LOG(WARNING) << "Ignoring RegisterValidateFunction() for flag pointer " << flag_ptr << ": no flag found at that address";
        return false;
    }
    else if (validate_fn_proto == flag->validate_function())
    {
        return true; // ok to register the same function over and over again
    }
    else if (validate_fn_proto != NULL && flag->validate_function() != NULL)
    {
        LOG(WARNING) << "Ignoring RegisterValidateFunction() for flag '" << flag->name() << "': validate-fn already registered";
        return false;
    }
    else
    {
        flag->validate_fn_proto_ = validate_fn_proto;
        return true;
    }
}

// --------------------------------------------------------------------
// RegisterFlagValidator()
//    RegisterFlagValidator() is the function that clients use to
//    'decorate' a flag with a validation function.  Once this is
//    done, every time the flag is set (including when the flag
//    is parsed from argv), the validator-function is called.
//       These functions return true if the validator was added
//    successfully, or false if not: the flag already has a validator,
//    (only one allowed per flag), the 1st arg isn't a flag, etc.
//       This function is not thread-safe.
// --------------------------------------------------------------------

bool RegisterFlagValidator(const bool * flag, bool (*validate_fn)(const char *, bool))
{
    return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const int32 * flag, bool (*validate_fn)(const char *, int32))
{
    return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const uint32 * flag, bool (*validate_fn)(const char *, uint32))
{
    return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const int64 * flag, bool (*validate_fn)(const char *, int64))
{
    return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const uint64 * flag, bool (*validate_fn)(const char *, uint64))
{
    return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const double * flag, bool (*validate_fn)(const char *, double))
{
    return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}
bool RegisterFlagValidator(const string * flag, bool (*validate_fn)(const char *, const string &))
{
    return AddFlagValidator(flag, reinterpret_cast<ValidateFnProto>(validate_fn));
}

} // namespace JFLAGS_NAMESPACE

