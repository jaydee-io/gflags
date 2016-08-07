////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "FlagRegisterer.h"
#include "FlagRegistry.h"
#include "FlagValue.h"
#include "CommandLineFlag.h"

namespace JFLAGS_NAMESPACE {

// --------------------------------------------------------------------
// FlagRegisterer
//    This class exists merely to have a global constructor (the
//    kind that runs before main(), that goes an initializes each
//    flag that's been declared.  Note that it's very important we
//    don't have a destructor that deletes flag_, because that would
//    cause us to delete current_storage/defvalue_storage as well,
//    which can cause a crash if anything tries to access the flag
//    values in a global destructor.
// --------------------------------------------------------------------

FlagRegisterer::FlagRegisterer(const char * name, const char * type, const char * help, const char * filename, void * current_storage, void * defvalue_storage)
{
    if (help == NULL)
        help = "";
    // FlagValue expects the type-name to not include any namespace
    // components, so we get rid of those, if any.
    if (strchr(type, ':'))
        type = strrchr(type, ':') + 1;
    FlagValue * current = new FlagValue(current_storage, type, false);
    FlagValue * defvalue = new FlagValue(defvalue_storage, type, false);
    // Importantly, flag_ will never be deleted, so storage is always good.
    CommandLineFlag * flag = new CommandLineFlag(name, help, filename, current, defvalue);
    FlagRegistry::GlobalRegistry()->RegisterFlag(flag); // default registry
}

} // namespace JFLAGS_NAMESPACE

