#!/bin/sh
set -e

header="$1"
shift
codelet="$1"

cat <<EOF
/* Generated automatically.  DO NOT EDIT! */
#define SIMD_HEADER "$header"
#include "../common/$codelet"
EOF
