AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/rdft	\
-I$(top_srcdir)/rdft/simd -I$(top_srcdir)/simd-support

EXTRA_DIST = $(SIMD_CODELETS) genus.c codlist.c

$(EXTRA_DIST): Makefile
	(							\
	echo "/* Generated automatically.  DO NOT EDIT! */";	\
	echo "#define SIMD_HEADER \"$(SIMD_HEADER)\"";		\
	echo "#include \"../common/"$*".c\"";			\
	) >$@

