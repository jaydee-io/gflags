////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
/* Author: Craig Silverstein
 */

#ifndef _WIN32
#error You should only be including windows/port.cc in a windows environment!
#endif

#include <assert.h>
#include <stdarg.h> // for va_list, va_start, va_end
#include <string.h> // for strlen(), memset(), memcmp()
#include <windows.h>

#include "windows_port.h"

// These call the windows _vsnprintf, but always NUL-terminate.
#if !defined(__MINGW32__) && !defined(__MINGW64__) /* mingw already defines */
#if !(defined(_MSC_VER) && _MSC_VER >= 1900)       /* msvc 2015 already defines */

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // ignore _vsnprintf security warning
#endif
int safe_vsnprintf(char * str, size_t size, const char * format, va_list ap)
{
    if (size == 0) // not even room for a \0?
        return -1; // not what C99 says to do, but what windows does
    str[size - 1] = '\0';
    return _vsnprintf(str, size - 1, format, ap);
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

int snprintf(char * str, size_t size, const char * format, ...)
{
    int r;
    va_list ap;
    va_start(ap, format);
    r = vsnprintf(str, size, format, ap);
    va_end(ap);
    return r;
}

#endif /* if !(defined(_MSC_VER) && _MSC_VER >= 1900)  */
#endif /* #if !defined(__MINGW32__) && !defined(__MINGW64__) */
