////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "FlagSaver.h"
#include "jflags_internals.h"

namespace JFLAGS_NAMESPACE {

// --------------------------------------------------------------------
// FlagSaver
// FlagSaverImpl
//    This class stores the states of all flags at construct time,
//    and restores all flags to that state at destruct time.
//    Its major implementation challenge is that it never modifies
//    pointers in the 'main' registry, so global FLAG_* vars always
//    point to the right place.
// --------------------------------------------------------------------

// Constructs an empty FlagSaverImpl object.
FlagSaverImpl::FlagSaverImpl(FlagRegistry * main_registry)
: main_registry_(main_registry)
{
}

FlagSaverImpl::~FlagSaverImpl()
{
    // reclaim memory from each of our CommandLineFlags
    vector<CommandLineFlag *>::const_iterator it;
    for (it = backup_registry_.begin(); it != backup_registry_.end(); ++it)
        delete *it;
}

// Saves the flag states from the flag registry into this object.
// It's an error to call this more than once.
// Must be called when the registry mutex is not held.
void FlagSaverImpl::SaveFromRegistry()
{
    FlagRegistryLock frl(main_registry_);
    assert(backup_registry_.empty()); // call only once!
    for (FlagRegistry::FlagConstIterator it = main_registry_->flags_.begin(); it != main_registry_->flags_.end(); ++it)
    {
        const CommandLineFlag * main = it->second;
        // Sets up all the const variables in backup correctly
        CommandLineFlag * backup = new CommandLineFlag(main->name(), main->help(), main->filename(), main->current_->New(), main->defvalue_->New());
        // Sets up all the non-const variables in backup correctly
        backup->CopyFrom(*main);
        backup_registry_.push_back(backup); // add it to a convenient list
    }
}

// Restores the saved flag states into the flag registry.  We
// assume no flags were added or deleted from the registry since
// the SaveFromRegistry; if they were, that's trouble!  Must be
// called when the registry mutex is not held.
void FlagSaverImpl::RestoreToRegistry()
{
    FlagRegistryLock frl(main_registry_);
    vector<CommandLineFlag *>::const_iterator it;
    for (it = backup_registry_.begin(); it != backup_registry_.end(); ++it)
    {
        CommandLineFlag * main = main_registry_->FindFlagLocked((*it)->name());
        if (main != NULL) // if NULL, flag got deleted from registry(!)
            main->CopyFrom(**it);
    }
}

FlagSaver::FlagSaver()
: impl_(new FlagSaverImpl(FlagRegistry::GlobalRegistry()))
{
    impl_->SaveFromRegistry();
}

FlagSaver::~FlagSaver()
{
    impl_->RestoreToRegistry();
    delete impl_;
}

} // namespace JFLAGS_NAMESPACE

