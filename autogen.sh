#!/bin/bash -e
############################################################################
#
# NOTE: If you just want to build FFTW, do not use this file.  Just use
# the ordinary ./configure && make commands as described in the installation
# section of the manual.
#
# This file is only for users that want to generate their own codelets,
# as described in the "generating your own code" section of the manual.
#
############################################################################

if test ! -f support/codelets.mk; then
    echo "Run python3 support/code_generation.py before autogen.sh." >&2
    exit 1
fi

test -f ChangeLog || touch ChangeLog
exec autoreconf --verbose --install --force "$@"
