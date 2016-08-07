////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------------------
// FlagRegistry is a collection of CommandLineFlags.  There's the
// global registry, which is where flags defined via DEFINE_foo()
// live.  But it's possible to define your own flag, manually, in a
// different registry you create.  (In practice, multiple registries
// are used only by FlagSaver).
//
// A given FlagValue is owned by exactly one CommandLineFlag.  A given
// CommandLineFlag is owned by exactly one FlagRegistry.  FlagRegistry
// has a lock; any operation that writes to a FlagValue or
// CommandLineFlag owned by that registry must acquire the
// FlagRegistry lock before doing so.
// --------------------------------------------------------------------
#ifndef JFLAGS_FLAG_REGISTRY_H_
#define JFLAGS_FLAG_REGISTRY_H_
#include "jflags_error.h"
#include "jflags_access.h"
#include "CommandLineFlag.h"
#include "mutex.h"

#include <map>
#include <string>
#include <vector>

using namespace MUTEX_NAMESPACE;

namespace JFLAGS_NAMESPACE {

using std::map;
using std::string;
using std::vector;

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
    friend class FlagSaverImpl; // reads all the flags in order
                                                  // to copy them
    friend class CommandLineFlagParser;           // for ValidateAllFlags
    friend void GetAllFlags(vector<CommandLineFlagInfo> *);

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

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_FLAG_REGISTRY_H_

