////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_FLAG_VALUE_H_
#define JFLAGS_FLAG_VALUE_H_

#include "jflags_validator.h" // For ValidateFnProto
#include "jflags_declare.h" // IWYU pragma: export
#include "util.h"

#include <string>

namespace JFLAGS_NAMESPACE {

using std::string;

// --------------------------------------------------------------------
// FlagValue
//    This represent the value a single flag might have.  The major
//    functionality is to convert from a string to an object of a
//    given type, and back.  Thread-compatible.
// --------------------------------------------------------------------

class CommandLineFlag;
class FlagValue
{
public:
    FlagValue(void * valbuf, const char * type, bool transfer_ownership_of_value);
    ~FlagValue();

    bool ParseFrom(const char * spec);
    string ToString() const;

private:
    friend class CommandLineFlag;                 // for many things, including Validate()
    friend class FlagSaverImpl; // calls New()
    friend class FlagRegistry;                    // checks value_buffer_ for flags_by_ptr_ map
    template <typename T>
    friend T GetFromEnv(const char *, const char *, T);
    friend bool TryParseLocked(const CommandLineFlag *, FlagValue *, const char *, string *); // for New(), CopyFrom()

    enum ValueType
    {
        FV_BOOL = 0,
        FV_INT32 = 1,
        FV_UINT32 = 2,
        FV_INT64 = 3,
        FV_UINT64 = 4,
        FV_DOUBLE = 5,
        FV_STRING = 6,
        FV_MAX_INDEX = 6,
    };
    const char * TypeName() const;
    bool Equal(const FlagValue & x) const;
    FlagValue * New() const; // creates a new one with default value
    void CopyFrom(const FlagValue & x);
    int ValueSize() const;

    // Calls the given validate-fn on value_buffer_, and returns
    // whatever it returns.  But first casts validate_fn_proto to a
    // function that takes our value as an argument (eg void
    // (*validate_fn)(bool) for a bool flag).
    bool Validate(const char * flagname, ValidateFnProto validate_fn_proto) const;

    void * value_buffer_; // points to the buffer holding our data
    int8 type_;           // how to interpret value_
    bool owns_value_;     // whether to free value on destruct

    FlagValue(const FlagValue &); // no copying!
    void operator=(const FlagValue &);
};


} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_FLAG_VALUE_H_

