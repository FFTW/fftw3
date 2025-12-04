#!/bin/sh
set -e

copyright_f="$1"
shift
twovers_f="$1"
shift
indent_f="$1"
shift
prelude_f="$1"
shift

(cat "$copyright_f" "$prelude_f"; $twovers_f $*) | sed -e s/@DATE@/"`date`"/ | $indent_f -kr -cs -i5 -l800 -fca -nfc1 -sc -sob -cli4 -TR -Tplanner -TV
