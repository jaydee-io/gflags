#!/bin/bash

################################################################################
#                                    jflags
#
# This file is distributed under the 3-clause Berkeley Software Distribution
# License. See LICENSE.txt for details.
################################################################################

REGEXP="@\([^ \t]*\)[ \t]@"

clang-format -i lib/* include/*
for FILE in $(grep -Rl "$REGEXP" include/* lib/*) ; do
    sed "s/$REGEXP/@\1@/g" $FILE > $FILE.modif
    mv $FILE.modif $FILE
done
