////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
// --------------------------------------------------------------------
// Some deprecated or hopefully-soon-to-be-deprecated functions.
#ifndef JFLAGS_DEPRECATED_H_
#define JFLAGS_DEPRECATED_H_

#include "jflags_declare.h" // IWYU pragma: export

namespace JFLAGS_NAMESPACE {

// This is often used for logging.  TODO(csilvers): figure out a better way
extern JFLAGS_DLL_DECL std::string CommandlineFlagsIntoString();
// Usually where this is used, a FlagSaver should be used instead.
extern JFLAGS_DLL_DECL bool ReadFlagsFromString(const std::string & flagfilecontents, const char * prog_name, bool errors_are_fatal); // uses SET_FLAGS_VALUE

// These let you manually implement --flagfile functionality.
// DEPRECATED.
extern JFLAGS_DLL_DECL bool AppendFlagsIntoFile(const std::string & filename, const char * prog_name);
extern JFLAGS_DLL_DECL bool ReadFromFlagsFile(const std::string & filename, const char * prog_name, bool errors_are_fatal); // uses SET_FLAGS_VALUE

} // namespace JFLAGS_NAMESPACE

#endif // JFLAGS_DEPRECATED_H_

