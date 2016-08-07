////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
#ifndef JFLAGS_INTERNALS_H_ 
#define JFLAGS_INTERNALS_H_

#include "jflags.h"
#include "jflags_error.h"
#include "config.h"

#include <ctype.h>
#include <string.h>

#include <algorithm>
#include <map>
#include <vector>

#include "FlagValue.h"
#include "CommandLineFlag.h"
#include "FlagRegistry.h"

namespace JFLAGS_NAMESPACE {

using std::map;
using std::sort;
using std::vector;

string ReadFileIntoString(const char * filename);

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_INTERNALS_H_
