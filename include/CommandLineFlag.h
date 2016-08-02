////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_COMMAND_LINE_FLAG_H_
#define JFLAGS_COMMAND_LINE_FLAG_H_
#include "FlagValue.h"

#include <string>

//#include "jflags_declare.h" // IWYU pragma: export

namespace JFLAGS_NAMESPACE {

using std::string;

// --------------------------------------------------------------------
// CommandLineFlag
//    This represents a single flag, including its name, description,
//    default value, and current value.  Mostly this serves as a
//    struct, though it also knows how to register itself.
//       All CommandLineFlags are owned by a (exactly one)
//    FlagRegistry.  If you wish to modify fields in this class, you
//    should acquire the FlagRegistry lock for the registry that owns
//    this flag.
// --------------------------------------------------------------------

class CommandLineFlag
{
public:
    // Note: we take over memory-ownership of current_val and default_val.
    CommandLineFlag(const char * name, const char * help, const char * filename, FlagValue * current_val, FlagValue * default_val);
    ~CommandLineFlag();

    const char * name() const { return name_; }
    const char * help() const { return help_; }
    const char * filename() const { return file_; }
    const char * CleanFileName() const; // nixes irrelevant prefix such as homedir
    string current_value() const { return current_->ToString(); }
    string default_value() const { return defvalue_->ToString(); }
    const char * type_name() const { return defvalue_->TypeName(); }
    ValidateFnProto validate_function() const { return validate_fn_proto_; }
    const void * flag_ptr() const { return current_->value_buffer_; }

    void FillCommandLineFlagInfo(struct CommandLineFlagInfo * result);

    // If validate_fn_proto_ is non-NULL, calls it on value, returns result.
    bool Validate(const FlagValue & value) const;
    bool ValidateCurrent() const { return Validate(*current_); }

private:
    // for SetFlagLocked() and setting flags_by_ptr_
    friend class FlagRegistry;
    friend class FlagSaverImpl; // for cloning the values
    // set validate_fn
    friend bool AddFlagValidator(const void *, ValidateFnProto);

    // This copies all the non-const members: modified, processed, defvalue, etc.
    void CopyFrom(const CommandLineFlag & src);

    void UpdateModifiedBit();

    const char * const name_; // Flag name
    const char * const help_; // Help message
    const char * const file_; // Which file did this come from?
    bool modified_;           // Set after default assignment?
    FlagValue * defvalue_;    // Default value for flag
    FlagValue * current_;     // Current value for flag
    // This is a casted, 'generic' version of validate_fn, which actually
    // takes a flag-value as an arg (void (*validate_fn)(bool), say).
    // When we pass this to current_->Validate(), it will cast it back to
    // the proper type.  This may be NULL to mean we have no validate_fn.
    ValidateFnProto validate_fn_proto_;

    CommandLineFlag(const CommandLineFlag &); // no copying!
    void operator=(const CommandLineFlag &);
};

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_COMMAND_LINE_FLAG_H_

