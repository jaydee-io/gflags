## jflags tests

# ----------------------------------------------------------------------------
# output directories
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

# set working directory of test commands
set (JFLAGS_FLAGFILES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# ----------------------------------------------------------------------------
# common include directories and link libraries
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories ("${jflags_SOURCE_DIR}/include")
include_directories ("${jflags_BINARY_DIR}/include")
include_directories ("${jflags_BINARY_DIR}/include/jflags")

if (BUILD_SHARED_LIBS)
  set (type shared)
  if (JFLAGS_IS_A_DLL)
    add_definitions(-DJFLAGS_IS_A_DLL)
  endif ()
else ()
  set (type static)
endif ()
if (BUILD_jflags_LIB)
  link_libraries (jflags_${type})
else ()
  link_libraries (jflags_nothreads_${type})
endif ()

# ----------------------------------------------------------------------------
# STRIP_FLAG_HELP
add_executable (jflags_strip_flags_test jflags_strip_flags_test.cc)
# Make sure the --help output doesn't print the stripped text.
add_jflags_test (strip_flags_help 1 "" "This text should be stripped out" jflags_strip_flags_test --help)
# Make sure the stripped text isn't in the binary at all.
add_test (
  NAME strip_flags_binary
  COMMAND "${CMAKE_COMMAND}" "-DBINARY=$<TARGET_FILE:jflags_strip_flags_test>"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/jflags_strip_flags_test.cmake"
)

# ----------------------------------------------------------------------------
# unit tests
configure_file (jflags_unittest.cc jflags_unittest-main.cc COPYONLY)
configure_file (jflags_unittest.cc jflags_unittest_main.cc COPYONLY)

add_executable (jflags_unittest      jflags_unittest.cc)
add_executable (jflags_unittest-main jflags_unittest-main.cc)
add_executable (jflags_unittest_main jflags_unittest_main.cc)

if (OS_WINDOWS)
  set (SLASH "\\\\")
else ()
  set (SLASH "/")
endif ()

# First, just make sure the  jflags_unittest  works as-is
add_jflags_test(unittest 0 "" "" jflags_unittest)

# --help should show all flags, including flags from jflags_reporting
add_jflags_test(help-reporting 1 "${SLASH}jflags_reporting.cc:" ""  jflags_unittest  --help)

# Make sure that --help prints even very long helpstrings.
add_jflags_test(long-helpstring 1 "end of a long helpstring" ""  jflags_unittest  --help)

# Make sure --help reflects flag changes made before flag-parsing
add_jflags_test(changed_bool1 1 "-changed_bool1 (changed) type: bool default: true" ""  jflags_unittest  --help)
add_jflags_test(changed_bool2 1 "-changed_bool2 (changed) type: bool default: false currently: true" ""  jflags_unittest  --help)
# And on the command-line, too
add_jflags_test(changeable_string_var 1 "-changeable_string_var () type: string default: \"1\" currently: \"2\"" ""  jflags_unittest  --changeable_string_var 2 --help)

# --nohelp and --help=false should be as if we didn't say anything
add_jflags_test(nohelp     0 "PASS" ""  jflags_unittest  --nohelp)
add_jflags_test(help=false 0 "PASS" ""  jflags_unittest  --help=false)

# --helpfull is the same as help
add_jflags_test(helpfull 1 "${SLASH}jflags_reporting.cc:" ""  jflags_unittest  --helpfull)

# --helpshort should show only flags from the  jflags_unittest  itself
add_jflags_test(helpshort 1 "${SLASH}jflags_unittest.cc:" "${SLASH}jflags_reporting.cc:"  jflags_unittest  --helpshort)

# --helpshort should show the tldflag we created in the  jflags_unittest  dir
add_jflags_test(helpshort-tldflag1 1 "tldflag1" "${SLASH}google.cc:"  jflags_unittest  --helpshort)
add_jflags_test(helpshort-tldflag2 1 "tldflag2" "${SLASH}google.cc:"  jflags_unittest  --helpshort)

# --helpshort should work if the main source file is suffixed with [_-]main
add_jflags_test(helpshort-main 1 "${SLASH}jflags_unittest-main.cc:" "${SLASH}jflags_reporting.cc:" jflags_unittest-main --helpshort)
add_jflags_test(helpshort_main 1 "${SLASH}jflags_unittest_main.cc:" "${SLASH}jflags_reporting.cc:" jflags_unittest_main --helpshort)

# --helpon needs an argument
add_jflags_test(helpon 1 "'--helpon' is missing its argument; flag description: show help on" ""  jflags_unittest  --helpon)
# --helpon argument indicates what file we'll show args from
add_jflags_test(helpon=jflags 1 "${SLASH}jflags.cc:" "${SLASH}jflags_unittest.cc:"  jflags_unittest  --helpon=jflags)
# another way of specifying the argument
add_jflags_test(helpon_jflags 1 "${SLASH}jflags.cc:" "${SLASH}jflags_unittest.cc:"  jflags_unittest  --helpon jflags)
# test another argument
add_jflags_test(helpon=jflags_unittest 1 "${SLASH}jflags_unittest.cc:" "${SLASH}jflags.cc:"  jflags_unittest  --helpon=jflags_unittest)

# helpmatch is like helpon but takes substrings
add_jflags_test(helpmatch_reporting 1 "${SLASH}jflags_reporting.cc:" "${SLASH}jflags_unittest.cc:"  jflags_unittest  -helpmatch reporting)
add_jflags_test(helpmatch=unittest  1 "${SLASH}jflags_unittest.cc:" "${SLASH}jflags.cc:"  jflags_unittest  -helpmatch=unittest)

# if no flags are found with helpmatch or helpon, suggest --help
add_jflags_test(helpmatch=nosuchsubstring 1 "No modules matched" "${SLASH}jflags_unittest.cc:"  jflags_unittest  -helpmatch=nosuchsubstring)
add_jflags_test(helpon=nosuchmodule       1 "No modules matched" "${SLASH}jflags_unittest.cc:"  jflags_unittest  -helpon=nosuchmodule)

# helppackage shows all the flags in the same dir as this unittest
# --help should show all flags, including flags from google.cc
add_jflags_test(helppackage 1 "${SLASH}jflags_reporting.cc:" ""  jflags_unittest  --helppackage)

# xml!
add_jflags_test(helpxml 1 "${SLASH}jflags_unittest.cc</file>" "${SLASH}jflags_unittest.cc:"  jflags_unittest  --helpxml)

# just print the version info and exit
add_jflags_test(version-1 0 "jflags_unittest"      "${SLASH}jflags_unittest.cc:"  jflags_unittest  --version)
add_jflags_test(version-2 0 "version test_version" "${SLASH}jflags_unittest.cc:"  jflags_unittest  --version)

# --undefok is a fun flag...
add_jflags_test(undefok-1 1 "unknown command line flag 'foo'" ""  jflags_unittest  --undefok= --foo --unused_bool)
add_jflags_test(undefok-2 0 "PASS" ""  jflags_unittest  --undefok=foo --foo --unused_bool)
# If you say foo is ok to be undefined, we'll accept --nofoo as well
add_jflags_test(undefok-3 0 "PASS" ""  jflags_unittest  --undefok=foo --nofoo --unused_bool)
# It's ok if the foo is in the middle
add_jflags_test(undefok-4 0 "PASS" ""  jflags_unittest  --undefok=fee,fi,foo,fum --foo --unused_bool)
# But the spelling has to be just right...
add_jflags_test(undefok-5 1 "unknown command line flag 'foo'" ""  jflags_unittest  --undefok=fo --foo --unused_bool)
add_jflags_test(undefok-6 1 "unknown command line flag 'foo'" ""  jflags_unittest  --undefok=foot --foo --unused_bool)

# See if we can successfully load our flags from the flagfile
add_jflags_test(flagfile.1 0 "jflags_unittest" "${SLASH}jflags_unittest.cc:"  jflags_unittest  "--flagfile=flagfile.1")
add_jflags_test(flagfile.2 0 "PASS" ""  jflags_unittest  "--flagfile=flagfile.2")
add_jflags_test(flagfile.3 0 "PASS" ""  jflags_unittest  "--flagfile=flagfile.3")

# Also try to load flags from the environment
add_jflags_test(fromenv=version      0 "jflags_unittest" "${SLASH}jflags_unittest.cc:"  jflags_unittest  --fromenv=version)
add_jflags_test(tryfromenv=version   0 "jflags_unittest" "${SLASH}jflags_unittest.cc:"  jflags_unittest  --tryfromenv=version)
add_jflags_test(fromenv=help         0 "PASS" ""  jflags_unittest  --fromenv=help)
add_jflags_test(tryfromenv=help      0 "PASS" ""  jflags_unittest  --tryfromenv=help)
add_jflags_test(fromenv=helpfull     1 "helpfull not found in environment" ""  jflags_unittest  --fromenv=helpfull)
add_jflags_test(tryfromenv=helpfull  0 "PASS" ""  jflags_unittest  --tryfromenv=helpfull)
add_jflags_test(tryfromenv=undefok   0 "PASS" ""  jflags_unittest  --tryfromenv=undefok --foo)
add_jflags_test(tryfromenv=weirdo    1 "unknown command line flag" ""  jflags_unittest  --tryfromenv=weirdo)
add_jflags_test(tryfromenv-multiple  0 "jflags_unittest" "${SLASH}jflags_unittest.cc:"  jflags_unittest  --tryfromenv=test_bool,version,unused_bool)
add_jflags_test(fromenv=test_bool    1 "not found in environment" ""  jflags_unittest  --fromenv=test_bool)
add_jflags_test(fromenv=test_bool-ok 1 "unknown command line flag" ""  jflags_unittest  --fromenv=test_bool,ok)
# Here, the --version overrides the fromenv
add_jflags_test(version-overrides-fromenv 0 "jflags_unittest" "${SLASH}jflags_unittest.cc:"  jflags_unittest  --fromenv=test_bool,version,ok)

# Make sure -- by itself stops argv processing
add_jflags_test(dashdash 0 "PASS" ""  jflags_unittest  -- --help)

# And we should die if the flag value doesn't pass the validator
add_jflags_test(always_fail 1 "ERROR: failed validation of new value 'true' for flag 'always_fail'" ""  jflags_unittest  --always_fail)

# And if locking in validators fails
# TODO(andreas): Worked on Windows 7 Release configuration, but causes
#                debugger abort() intervention in case of Debug configuration.
#add_jflags_test(deadlock_if_cant_lock 0 "PASS" ""  jflags_unittest  --deadlock_if_cant_lock)

# ----------------------------------------------------------------------------
# use jflags_declare.h
add_executable (jflags_declare_test jflags_declare_test.cc jflags_declare_flags.cc)

add_test(NAME jflags_declare COMMAND jflags_declare_test --message "Hello jflags!")
set_tests_properties(jflags_declare PROPERTIES PASS_REGULAR_EXPRESSION "Hello jflags!")

# ----------------------------------------------------------------------------
# configure Python script which configures and builds a test project
if (BUILD_NC_TESTS OR BUILD_CONFIG_TESTS)
  find_package (PythonInterp)
  if (NOT PYTHON_EXECUTABLE)
    message (FATAL_ERROR "No Python installation found! It is required by the (negative) compilation tests."
                         " Either install Python or set BUILD_NC_TESTS and BUILD_CONFIG_TESTS to FALSE.")
  endif ()
  set (TMPDIR "${PROJECT_BINARY_DIR}/Testing/Temporary")
  configure_file (jflags_build.py.in "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/build.py" @ONLY)
  function (add_jflags_build_test name srcdir expect_fail)
    set (srcdir "${CMAKE_CURRENT_SOURCE_DIR}/${srcdir}")
    add_test (
      NAME    "${name}"
      COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/build.py" 
                    ${name} ${srcdir} ${expect_fail}
    )
  endfunction ()
endif ()

# ----------------------------------------------------------------------------
# negative compilation tests
option (BUILD_NC_TESTS "Request addition of negative compilation tests." OFF)
mark_as_advanced (BUILD_NC_TESTS)
if (BUILD_NC_TESTS)
  add_jflags_build_test (nc_sanity               nc 0)
  add_jflags_build_test (nc_swapped_args         nc 1)
  add_jflags_build_test (nc_int_instead_of_bool  nc 1)
  add_jflags_build_test (nc_bool_in_quotes       nc 1)
  add_jflags_build_test (nc_define_string_with_0 nc 1)
endif ()

# ----------------------------------------------------------------------------
# build configuration test
option (BUILD_CONFIG_TESTS "Request addition of package configuration tests." OFF)
mark_as_advanced (BUILD_CONFIG_TESTS)
if (BUILD_CONFIG_TESTS)
  add_jflags_build_test (cmake_config config 0)
endif ()
