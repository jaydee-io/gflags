////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------------------
// FlagRegisterer is the helper class used by the DEFINE_* macros to
// allow work to be done at global initialization time.
// --------------------------------------------------------------------
#ifndef JFLAGS_FLAG_REGISTERER_H_
#define JFLAGS_FLAG_REGISTERER_H_

#include "jflags_declare.h" // IWYU pragma: export

namespace JFLAGS_NAMESPACE {

class JFLAGS_DLL_DECL FlagRegisterer
{
public:
    FlagRegisterer(const char * name, const char * type, const char * help, const char * filename, void * current_storage, void * defvalue_storage);
};

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_FLAG_REGISTERER_H_

