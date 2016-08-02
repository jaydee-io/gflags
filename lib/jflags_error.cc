////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#include "jflags_error.h"
#include <stdarg.h> // For va_list and related operations
#include <stdio.h>
#include <stdlib.h>

namespace JFLAGS_NAMESPACE {

// This is used by the unittest to test error-exit code
void JFLAGS_DLL_DECL (*jflags_exitfunc)(int) = &exit; // from stdlib.h

const char kError[] = "ERROR: ";

// Report Error and exit if requested.
void ReportError(DieWhenReporting should_die, const char * format, ...)
{
    char error_message[255];
    va_list ap;
    va_start(ap, format);
    vsnprintf(error_message, sizeof(error_message), format, ap);
    va_end(ap);
    fprintf(stderr, "%s", error_message);
    fflush(stderr); // should be unnecessary, but cygwin's rxvt buffers stderr
    if (should_die == DIE)
        jflags_exitfunc(1);
}

} // namespace JFLAGS_NAMESPACE

