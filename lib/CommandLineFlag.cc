////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "CommandLineFlag.h"
#include "jflags_infos.h"

namespace JFLAGS_NAMESPACE {

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

CommandLineFlag::CommandLineFlag(const char * name, const char * help, const char * filename, FlagValue * current_val, FlagValue * default_val)
: name_(name), help_(help), file_(filename), modified_(false), defvalue_(default_val), current_(current_val), validate_fn_proto_(NULL)
{
}

CommandLineFlag::~CommandLineFlag()
{
    delete current_;
    delete defvalue_;
}

const char * CommandLineFlag::CleanFileName() const
{
    // Compute top-level directory & file that this appears in
    // search full path backwards.
    // Stop going backwards at kRootDir; and skip by the first slash.
    static const char kRootDir[] = ""; // can set this to root directory,

    if (sizeof(kRootDir) - 1 == 0) // no prefix to strip
        return filename();

    const char * clean_name = filename() + strlen(filename()) - 1;
    while (clean_name > filename())
    {
        if (*clean_name == PATH_SEPARATOR)
        {
            if (strncmp(clean_name, kRootDir, sizeof(kRootDir) - 1) == 0)
            {
                clean_name += sizeof(kRootDir) - 1; // past root-dir
                break;
            }
        }
        --clean_name;
    }
    while (*clean_name == PATH_SEPARATOR)
        ++clean_name; // Skip any slashes
    return clean_name;
}

void CommandLineFlag::FillCommandLineFlagInfo(CommandLineFlagInfo * result)
{
    result->name = name();
    result->type = type_name();
    result->description = help();
    result->current_value = current_value();
    result->default_value = default_value();
    result->filename = CleanFileName();
    UpdateModifiedBit();
    result->is_default = !modified_;
    result->has_validator_fn = validate_function() != NULL;
    result->flag_ptr = flag_ptr();
}

void CommandLineFlag::UpdateModifiedBit()
{
    // Update the "modified" bit in case somebody bypassed the
    // Flags API and wrote directly through the FLAGS_name variable.
    if (!modified_ && !current_->Equal(*defvalue_))
        modified_ = true;
}

void CommandLineFlag::CopyFrom(const CommandLineFlag & src)
{
    // Note we only copy the non-const members; others are fixed at construct time
    if (modified_ != src.modified_)
        modified_ = src.modified_;
    if (!current_->Equal(*src.current_))
        current_->CopyFrom(*src.current_);
    if (!defvalue_->Equal(*src.defvalue_))
        defvalue_->CopyFrom(*src.defvalue_);
    if (validate_fn_proto_ != src.validate_fn_proto_)
        validate_fn_proto_ = src.validate_fn_proto_;
}

bool CommandLineFlag::Validate(const FlagValue & value) const
{
    if (validate_function() == NULL)
        return true;
    else
        return value.Validate(name(), validate_function());
}

} // namespace JFLAGS_NAMESPACE

