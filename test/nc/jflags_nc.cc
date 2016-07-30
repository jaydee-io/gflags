////////////////////////////////////////////////////////////////////////////////
//                                    jflags
//
// This file is distributed under the 3-clause Berkeley Software Distribution
// License. See LICENSE.txt for details.
////////////////////////////////////////////////////////////////////////////////
// ---
//
// A negative comiple test for jflags.

#include <jflags/jflags.h>

#if defined(TEST_NC_SWAPPED_ARGS)

DEFINE_bool(some_bool_flag,
            "the default value should go here, not the description",
            false);


#elif defined(TEST_NC_INT_INSTEAD_OF_BOOL)

DEFINE_bool(some_bool_flag_2,
            0,
            "should have been an int32 flag but mistakenly used bool instead");

#elif defined(TEST_NC_BOOL_IN_QUOTES)


DEFINE_bool(some_bool_flag_3,
            "false",
            "false in in quotes, which is wrong");

#elif defined(TEST_NC_SANITY)

DEFINE_bool(some_bool_flag_4,
            true,
            "this is the correct usage of DEFINE_bool");

#elif defined(TEST_NC_DEFINE_STRING_WITH_0)

DEFINE_string(some_string_flag,
              0,
              "Trying to construct a string by passing 0 would cause a crash.");

#endif

int main(int, char **)
{
  return 0;
}
