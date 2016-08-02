////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_ERROR_H_
#define JFLAGS_ERROR_H_

#include "jflags_declare.h" // IWYU pragma: export

namespace JFLAGS_NAMESPACE {

// This is used by the unittest to test error-exit code
extern void JFLAGS_DLL_DECL (*jflags_exitfunc)(int);

// Whether we should die when reporting an error.
enum DieWhenReporting
{
    DIE,
    DO_NOT_DIE
};

// Report Error and exit if requested.
void ReportError(DieWhenReporting should_die, const char * format, ...);

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_INFOS_H_

