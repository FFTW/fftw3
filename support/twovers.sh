#! /bin/sh

# wrapper to generate two codelet versions, with and without
# fma

genfft=$1
shift

echo "#ifdef HAVE_FMA"
echo
  $genfft -fma -scheduler-hack $*
echo
echo "#else /* HAVE_FMA */"
echo
  $genfft $*
echo
echo "#endif /* HAVE_FMA */"
