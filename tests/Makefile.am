AM_CPPFLAGS = -I$(top_srcdir)/kernel -I$(top_srcdir)/codelets	\
-I$(top_srcdir)/libbench -I$(top_srcdir)/dft
noinst_PROGRAMS = bench

bench_SOURCES = bench.c
bench_LDADD = ../codelets/libcodelets.la ../dft/libdft.la	\
../kernel/libkernel.la ../libbench/libbench.la