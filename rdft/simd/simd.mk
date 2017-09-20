AM_CPPFLAGS = -I $(top_srcdir)
EXTRA_DIST = $(SIMD_CODELETS) genus.c codlist.c

$(EXTRA_DIST): Makefile
	(							\
	echo "/* Generated automatically.  DO NOT EDIT! */";	\
	echo "#define SIMD_HEADER \"$(SIMD_HEADER)\"";		\
	echo "#include \"../common/"$*".c\"";			\
	) >$@

