#!/bin/bash

REGEXP="@\([^ \t]*\)[ \t]@"

clang-format -i lib/* include/*
for FILE in $(grep -Rl "$REGEXP" include/* lib/*) ; do
    sed "s/$REGEXP/@\1@/g" $FILE > $FILE.modif
    mv $FILE.modif $FILE
done
