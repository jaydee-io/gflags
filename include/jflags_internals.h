////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_INTERNALS_H_ 
#define JFLAGS_INTERNALS_H_

#include "jflags.h"
#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#if defined(HAVE_FNMATCH_H)
#include <fnmatch.h>
#elif defined(HAVE_SHLWAPI_H)
#include <shlwapi.h>
#endif
#include <stdarg.h> // For va_list and related operations
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <string>
#include <utility> // for pair<>
#include <vector>

#include "mutex.h"
#include "util.h"

using namespace MUTEX_NAMESPACE;

namespace JFLAGS_NAMESPACE {

using std::map;
using std::pair;
using std::sort;
using std::string;
using std::vector;

// This is a 'prototype' validate-function.  'Real' validate
// functions, take a flag-value as an argument: ValidateFn(bool) or
// ValidateFn(uint64).  However, for easier storage, we strip off this
// argument and then restore it when actually calling the function on
// a flag value.
typedef bool (*ValidateFnProto)();

// Whether we should die when reporting an error.
enum DieWhenReporting
{
    DIE,
    DO_NOT_DIE
};

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
    friend class JFLAGS_NAMESPACE::FlagSaverImpl; // calls New()
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
    friend class JFLAGS_NAMESPACE::FlagSaverImpl; // for cloning the values
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

// --------------------------------------------------------------------
// FlagRegistry
//    A FlagRegistry singleton object holds all flag objects indexed
//    by their names so that if you know a flag's name (as a C
//    string), you can access or set it.  If the function is named
//    FooLocked(), you must own the registry lock before calling
//    the function; otherwise, you should *not* hold the lock, and
//    the function will acquire it itself if needed.
// --------------------------------------------------------------------

struct StringCmp
{ // Used by the FlagRegistry map class to compare char*'s
    bool operator()(const char * s1, const char * s2) const
    {
        return (strcmp(s1, s2) < 0);
    }
};

class FlagRegistry
{
public:
    FlagRegistry() {}
    ~FlagRegistry()
    {
        // Not using STLDeleteElements as that resides in util and this
        // class is base.
        for (FlagMap::iterator p = flags_.begin(), e = flags_.end(); p != e; ++p) {
            CommandLineFlag * flag = p->second;
            delete flag;
        }
    }

    static void DeleteGlobalRegistry()
    {
        delete global_registry_;
        global_registry_ = NULL;
    }

    // Store a flag in this registry.  Takes ownership of the given pointer.
    void RegisterFlag(CommandLineFlag * flag);

    void Lock() { lock_.Lock(); }
    void Unlock() { lock_.Unlock(); }

    // Returns the flag object for the specified name, or NULL if not found.
    CommandLineFlag * FindFlagLocked(const char * name);

    // Returns the flag object whose current-value is stored at flag_ptr.
    // That is, for whom current_->value_buffer_ == flag_ptr
    CommandLineFlag * FindFlagViaPtrLocked(const void * flag_ptr);

    // A fancier form of FindFlag that works correctly if name is of the
    // form flag=value.  In that case, we set key to point to flag, and
    // modify v to point to the value (if present), and return the flag
    // with the given name.  If the flag does not exist, returns NULL
    // and sets error_message.
    CommandLineFlag * SplitArgumentLocked(const char * argument, string * key, const char ** v, string * error_message);

    // Set the value of a flag.  If the flag was successfully set to
    // value, set msg to indicate the new flag-value, and return true.
    // Otherwise, set msg to indicate the error, leave flag unchanged,
    // and return false.  msg can be NULL.
    bool SetFlagLocked(CommandLineFlag * flag, const char * value, FlagSettingMode set_mode, string * msg);

    static FlagRegistry * GlobalRegistry(); // returns a singleton registry

private:
    friend class JFLAGS_NAMESPACE::FlagSaverImpl; // reads all the flags in order
                                                  // to copy them
    friend class CommandLineFlagParser;           // for ValidateAllFlags
    friend void JFLAGS_NAMESPACE::GetAllFlags(vector<CommandLineFlagInfo> *);

    // The map from name to flag, for FindFlagLocked().
    typedef map<const char *, CommandLineFlag *, StringCmp> FlagMap;
    typedef FlagMap::iterator FlagIterator;
    typedef FlagMap::const_iterator FlagConstIterator;
    FlagMap flags_;

    // The map from current-value pointer to flag, fo FindFlagViaPtrLocked().
    typedef map<const void *, CommandLineFlag *> FlagPtrMap;
    FlagPtrMap flags_by_ptr_;

    static FlagRegistry * global_registry_; // a singleton registry

    Mutex lock_;
    static Mutex global_registry_lock_;

    static void InitGlobalRegistry();

    // Disallow
    FlagRegistry(const FlagRegistry &);
    FlagRegistry & operator=(const FlagRegistry &);
};

class FlagRegistryLock
{
public:
    explicit FlagRegistryLock(FlagRegistry * fr) : fr_(fr) { fr_->Lock(); }
    ~FlagRegistryLock() { fr_->Unlock(); }

private:
    FlagRegistry * const fr_;
};

// --------------------------------------------------------------------
// CommandLineFlagParser
//    Parsing is done in two stages.  In the first, we go through
//    argv.  For every flag-like arg we can make sense of, we parse
//    it and set the appropriate FLAGS_* variable.  For every flag-
//    like arg we can't make sense of, we store it in a vector,
//    along with an explanation of the trouble.  In stage 2, we
//    handle the 'reporting' flags like --help and --mpm_version.
//    (This is via a call to HandleCommandLineHelpFlags(), in
//    jflags_reporting.cc.)
//    An optional stage 3 prints out the error messages.
//       This is a bit of a simplification.  For instance, --flagfile
//    is handled as soon as it's seen in stage 1, not in stage 2.
// --------------------------------------------------------------------

class CommandLineFlagParser
{
public:
    // The argument is the flag-registry to register the parsed flags in
    explicit CommandLineFlagParser(FlagRegistry * reg) : registry_(reg) {}
    ~CommandLineFlagParser() {}

    // Stage 1: Every time this is called, it reads all flags in argv.
    // However, it ignores all flags that have been successfully set
    // before.  Typically this is only called once, so this 'reparsing'
    // behavior isn't important.  It can be useful when trying to
    // reparse after loading a dll, though.
    uint32 ParseNewCommandLineFlags(int * argc, char *** argv, bool remove_flags);

    // Stage 2: print reporting info and exit, if requested.
    // In jflags_reporting.cc:HandleCommandLineHelpFlags().

    // Stage 3: validate all the commandline flags that have validators
    // registered.
    void ValidateAllFlags();

    // Stage 4: report any errors and return true if any were found.
    bool ReportErrors();

    // Set a particular command line option.  "newval" is a string
    // describing the new value that the option has been set to.  If
    // option_name does not specify a valid option name, or value is not
    // a valid value for option_name, newval is empty.  Does recursive
    // processing for --flagfile and --fromenv.  Returns the new value
    // if everything went ok, or empty-string if not.  (Actually, the
    // return-string could hold many flag/value pairs due to --flagfile.)
    // NB: Must have called registry_->Lock() before calling this function.
    string ProcessSingleOptionLocked(CommandLineFlag * flag, const char * value, FlagSettingMode set_mode);

    // Set a whole batch of command line options as specified by contentdata,
    // which is in flagfile format (and probably has been read from a flagfile).
    // Returns the new value if everything went ok, or empty-string if
    // not.  (Actually, the return-string could hold many flag/value
    // pairs due to --flagfile.)
    // NB: Must have called registry_->Lock() before calling this function.
    string ProcessOptionsFromStringLocked(const string & contentdata, FlagSettingMode set_mode);

    // These are the 'recursive' flags, defined at the top of this file.
    // Whenever we see these flags on the commandline, we must take action.
    // These are called by ProcessSingleOptionLocked and, similarly, return
    // new values if everything went ok, or the empty-string if not.
    string ProcessFlagfileLocked(const string & flagval, FlagSettingMode set_mode);
    // diff fromenv/tryfromenv
    string ProcessFromenvLocked(const string & flagval, FlagSettingMode set_mode, bool errors_are_fatal);

private:
    FlagRegistry * const registry_;
    map<string, string> error_flags_; // map from name to error message
    // This could be a set<string>, but we reuse the map to minimize the .o size
    map<string, string> undefined_names_; // --[flag] name was not registered
};

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_INTERNALS_H_