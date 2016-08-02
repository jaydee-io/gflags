////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "FlagRegistry.h"

#include <utility> // for pair<>

namespace JFLAGS_NAMESPACE {

using std::pair;

// --------------------------------------------------------------------
// FlagRegistry
//    A FlagRegistry singleton object holds all flag objects indexed
//    by their names so that if you know a flag's name (as a C
//    string), you can access or set it.  If the function is named
//    FooLocked(), you must own the registry lock before calling
//    the function; otherwise, you should *not* hold the lock, and
//    the function will acquire it itself if needed.
// --------------------------------------------------------------------

void FlagRegistry::RegisterFlag(CommandLineFlag * flag)
{
    Lock();
    pair<FlagIterator, bool> ins = flags_.insert(pair<const char *, CommandLineFlag *>(flag->name(), flag));
    if (ins.second == false)
    { // means the name was already in the map
        if (strcmp(ins.first->second->filename(), flag->filename()) != 0)
        {
            ReportError(DIE, "ERROR: flag '%s' was defined more than once (in files '%s' and '%s').\n",
                        flag->name(), ins.first->second->filename(),
                        flag->filename());
        }
        else
        {
            ReportError(DIE,
                        "ERROR: something wrong with flag '%s' in file '%s'.  "
                        "One possibility: file '%s' is being linked both statically "
                        "and dynamically into this executable.\n",
                        flag->name(), flag->filename(), flag->filename());
        }
    }
    // Also add to the flags_by_ptr_ map.
    flags_by_ptr_[flag->current_->value_buffer_] = flag;
    Unlock();
}

CommandLineFlag * FlagRegistry::FindFlagLocked(const char * name)
{
    FlagConstIterator i = flags_.find(name);
    if (i == flags_.end())
        return NULL;
    else
        return i->second;
}

CommandLineFlag * FlagRegistry::FindFlagViaPtrLocked(const void * flag_ptr)
{
    FlagPtrMap::const_iterator i = flags_by_ptr_.find(flag_ptr);
    if (i == flags_by_ptr_.end())
        return NULL;
    else
        return i->second;
}

CommandLineFlag * FlagRegistry::SplitArgumentLocked(const char * arg, string * key, const char ** v, string * error_message)
{
    // Find the flag object for this option
    const char * flag_name;
    const char * value = strchr(arg, '=');
    if (value == NULL)
    {
        key->assign(arg);
        *v = NULL;
    }
    else
    {
        // Strip out the "=value" portion from arg
        key->assign(arg, value - arg);
        *v = ++value; // advance past the '='
    }
    flag_name = key->c_str();

    CommandLineFlag * flag = FindFlagLocked(flag_name);

    if (flag == NULL)
    {
        // If we can't find the flag-name, then we should return an error.
        // The one exception is if 1) the flag-name is 'nox', 2) there
        // exists a flag named 'x', and 3) 'x' is a boolean flag.
        // In that case, we want to return flag 'x'.
        if (!(flag_name[0] == 'n' && flag_name[1] == 'o'))
        {
            // flag-name is not 'nox', so we're not in the exception case.
            *error_message = StringPrintf("%sunknown command line flag '%s'\n", kError, key->c_str());
            return NULL;
        }
        flag = FindFlagLocked(flag_name + 2);
        if (flag == NULL)
        {
            // No flag named 'x' exists, so we're not in the exception case.
            *error_message = StringPrintf("%sunknown command line flag '%s'\n", kError, key->c_str());
            return NULL;
        }
        if (strcmp(flag->type_name(), "bool") != 0)
        {
            // 'x' exists but is not boolean, so we're not in the exception case.
            *error_message = StringPrintf("%sboolean value (%s) specified for %s command line flag\n", kError, key->c_str(), flag->type_name());
            return NULL;
        }
        // We're in the exception case!
        // Make up a fake value to replace the "no" we stripped out
        key->assign(flag_name + 2); // the name without the "no"
        *v = "0";
    }

    // Assign a value if this is a boolean flag
    if (*v == NULL && strcmp(flag->type_name(), "bool") == 0)
        *v = "1"; // the --nox case was already handled, so this is the --x case

    return flag;
}

bool TryParseLocked(const CommandLineFlag * flag, FlagValue * flag_value, const char * value, string * msg)
{
    // Use tenative_value, not flag_value, until we know value is valid.
    FlagValue * tentative_value = flag_value->New();
    if (!tentative_value->ParseFrom(value))
    {
        if (msg)
            StringAppendF(msg, "%sillegal value '%s' specified for %s flag '%s'\n", kError, value, flag->type_name(), flag->name());
        delete tentative_value;
        return false;
    }
    else if (!flag->Validate(*tentative_value))
    {
        if (msg)
            StringAppendF(msg, "%sfailed validation of new value '%s' for flag '%s'\n", kError, tentative_value->ToString().c_str(), flag->name());
        delete tentative_value;
        return false;
    }
    else
    {
        flag_value->CopyFrom(*tentative_value);
        if (msg)
            StringAppendF(msg, "%s set to %s\n", flag->name(), flag_value->ToString().c_str());
        delete tentative_value;
        return true;
    }
}

bool FlagRegistry::SetFlagLocked(CommandLineFlag * flag, const char * value, FlagSettingMode set_mode, string * msg)
{
    flag->UpdateModifiedBit();
    switch (set_mode)
    {
        case SET_FLAGS_VALUE:
        {
            // set or modify the flag's value
            if (!TryParseLocked(flag, flag->current_, value, msg))
                return false;
            flag->modified_ = true;
            break;
        }
        case SET_FLAG_IF_DEFAULT:
        {
            // set the flag's value, but only if it hasn't been set by someone else
            if (!flag->modified_)
            {
                if (!TryParseLocked(flag, flag->current_, value, msg))
                    return false;
                flag->modified_ = true;
            }
            else
            {
                *msg = StringPrintf("%s set to %s", flag->name(), flag->current_value().c_str());
            }
            break;
        }
        case SET_FLAGS_DEFAULT:
        {
            // modify the flag's default-value
            if (!TryParseLocked(flag, flag->defvalue_, value, msg))
                return false;
            if (!flag->modified_)
                // Need to set both defvalue *and* current, in this case
                TryParseLocked(flag, flag->current_, value, NULL);
            break;
        }
        default:
        {
            // unknown set_mode
            assert(false);
            return false;
        }
    }

    return true;
}

// Get the singleton FlagRegistry object
FlagRegistry * FlagRegistry::global_registry_ = NULL;
Mutex FlagRegistry::global_registry_lock_(Mutex::LINKER_INITIALIZED);

FlagRegistry * FlagRegistry::GlobalRegistry()
{
    MutexLock acquire_lock(&global_registry_lock_);
    if (!global_registry_)
        global_registry_ = new FlagRegistry;
    return global_registry_;
}

} // namespace JFLAGS_NAMESPACE

