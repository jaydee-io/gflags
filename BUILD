# Bazel build file for jflags
#
# See INSTALL.md for instructions for adding jflags to a Bazel workspace.

licenses(["notice"])

cc_library(
    name = "jflags",
    srcs = [
        "lib/jflags.cc",
        "lib/jflags_completions.cc",
        "lib/jflags_reporting.cc",
        "include/mutex.h",
        "include/util.h",
        ":config_h",
        ":jflags_completions_h",
        ":jflags_declare_h",
        ":jflags_h",
        ":includes",
    ],
    hdrs = ["jflags.h"],
    copts = [
        # The config.h gets generated to the package directory of
        # GENDIR, and we don't want to put it into the includes
        # otherwise the dependent may pull it in by accident.
        "-I$(GENDIR)/" + PACKAGE_NAME,
        "-Wno-sign-compare",
        "-DHAVE_STDINT_H",
        "-DHAVE_SYS_TYPES_H",
        "-DHAVE_INTTYPES_H",
        "-DHAVE_SYS_STAT_H",
        "-DHAVE_UNISTD_H",
        "-DHAVE_FNMATCH_H",
        "-DHAVE_STRTOLL",
        "-DHAVE_STRTOQ",
        "-DHAVE_PTHREAD",
        "-DHAVE_RWLOCK",
        "-DJFLAGS_INTTYPES_FORMAT_C99",
    ],
    includes = [
        "include",
    ],
    visibility = ["//visibility:public"],
)

genrule(
    name = "config_h",
    srcs = [
        "include/config.h.in",
    ],
    outs = [
        "config.h",
    ],
    cmd = "awk '{ gsub(/^#cmakedefine/, \"//cmakedefine\"); print; }' $(<) > $(@)",
)

genrule(
    name = "jflags_h",
    srcs = [
        "include/jflags.h.in",
    ],
    outs = [
        "jflags.h",
    ],
    cmd = "awk '{ gsub(/@(JFLAGS_ATTRIBUTE_UNUSED|INCLUDE_JFLAGS_NS_H)@/, \"\"); print; }' $(<) > $(@)",
)

genrule(
    name = "jflags_completions_h",
    srcs = [
        "include/jflags_completions.h.in",
    ],
    outs = [
        "jflags_completions.h",
    ],
    cmd = "awk '{ gsub(/@JFLAGS_NAMESPACE@/, \"jflags\"); print; }' $(<) > $(@)",
)

genrule(
    name = "jflags_declare_h",
    srcs = [
        "include/jflags_declare.h.in",
    ],
    outs = [
        "jflags_declare.h",
    ],
    cmd = ("awk '{ " +
           "gsub(/@JFLAGS_NAMESPACE@/, \"jflags\"); " +
           "gsub(/@(HAVE_STDINT_H|HAVE_SYS_TYPES_H|HAVE_INTTYPES_H|JFLAGS_INTTYPES_FORMAT_C99)@/, \"1\"); " +
           "gsub(/@([A-Z0-9_]+)@/, \"0\"); " +
           "print; }' $(<) > $(@)"),
)

genrule(
    name = "includes",
    srcs = [
        ":jflags_h",
        ":jflags_declare_h",
    ],
    outs = [
        "include/jflags/jflags.h",
        "include/jflags/jflags_declare.h",
    ],
    cmd = "mkdir -p $(@D)/include/jflags && cp $(SRCS) $(@D)/include/jflags",
)
