# this makefile requires GNU make.

EXTRA_DIST = .depend algsimp.ml annotate.ml assoctable.ml c.ml		\
complex.ml conv.ml dag.ml expr.ml fft.ml gen_athnotw.ml gen_athtw.ml	\
gen_conv.ml gen_hc2hc.ml gen_hc2r.ml gen_mdct.ml gen_notw.ml		\
gen_notw_c.ml gen_r2hc.ml gen_r2r.ml gen_twiddle.ml gen_twiddle_c.ml	\
gen_twidsq.ml gen_twidsq_c.ml genutil.ml littlesimp.ml magic.ml		\
monads.ml number.ml oracle.ml schedule.ml simd.ml simdmagic.ml		\
to_alist.ml trig.ml twiddle.ml unique.ml util.ml variable.ml		\
algsimp.mli annotate.mli assoctable.mli c.mli complex.mli conv.mli	\
dag.mli expr.mli fft.mli littlesimp.mli number.mli oracle.mli		\
schedule.mli simd.mli to_alist.mli trig.mli twiddle.mli unique.mli	\
util.mli variable.mli

noinst_SCRIPTS=gen_notw gen_notw_c gen_twiddle gen_twiddle_c		\
gen_twidsq gen_twidsq_c gen_r2r gen_conv gen_r2hc gen_hc2r gen_hc2hc	\
gen_mdct

libgenfft_objects = unique.cmo magic.cmo util.cmo number.cmo		\
variable.cmo expr.cmo littlesimp.cmo monads.cmo complex.cmo		\
assoctable.cmo oracle.cmo algsimp.cmo simdmagic.cmo to_alist.cmo	\
dag.cmo schedule.cmo annotate.cmo twiddle.cmo fft.cmo conv.cmo c.cmo	\
simd.cmo genutil.cmo trig.cmo

libgenfft.cma: $(libgenfft_objects)
	$(OCAMLC) -a -o $@ $^

libgenfft.cmxa libgenfft.a: $(libgenfft_objects:.cmo=.cmx)
	$(OCAMLOPT) -a -o libgenfft.cmxa $^
	$(RANLIB) libgenfft.a

gen_notw_OBJECTS = gen_notw.cmo
gen_notw: libgenfft.cmxa $(gen_notw_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_notw

gen_notw_c_OBJECTS = gen_notw_c.cmo
gen_notw_c: libgenfft.cmxa $(gen_notw_c_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_notw_c

gen_twiddle_OBJECTS = gen_twiddle.cmo
gen_twiddle: libgenfft.cmxa $(gen_twiddle_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_twiddle

gen_twiddle_c_OBJECTS = gen_twiddle_c.cmo
gen_twiddle_c: libgenfft.cmxa $(gen_twiddle_c_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_twiddle_c

gen_twidsq_OBJECTS = gen_twidsq.cmo
gen_twidsq: libgenfft.cmxa $(gen_twidsq_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_twidsq

gen_twidsq_c_OBJECTS = gen_twidsq_c.cmo
gen_twidsq_c: libgenfft.cmxa $(gen_twidsq_c_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_twidsq_c

gen_r2hc_OBJECTS = gen_r2hc.cmo
gen_r2hc: libgenfft.cmxa $(gen_r2hc_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_r2hc

gen_hc2r_OBJECTS = gen_hc2r.cmo
gen_hc2r: libgenfft.cmxa $(gen_hc2r_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_hc2r

gen_hc2hc_OBJECTS = gen_hc2hc.cmo
gen_hc2hc: libgenfft.cmxa $(gen_hc2hc_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_hc2hc

gen_r2r_OBJECTS = gen_r2r.cmo
gen_r2r: libgenfft.cmxa $(gen_r2r_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_r2r

# experimental stuff
gen_conv_OBJECTS = gen_conv.cmo
gen_conv: libgenfft.cmxa $(gen_conv_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_conv

gen_mdct_OBJECTS = gen_mdct.cmo
gen_mdct: libgenfft.cmxa $(gen_mdct_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
maintainer-clean-local::
	rm -f gen_mdct


SUFFIXES = .mli .ml .cmi .cmo .cmx .p.cmx .c .o .cma .cmxa .a

OCAMLC=@OCAMLC@
OCAMLOPT=@OCAMLOPT@
OCAMLDEP=@OCAMLDEP@
OCAMLCFLAGS=@OCAMLCFLAGS@
OCAMLOPTCFLAGS=@OCAMLOPTCFLAGS@
OCAMLLDFLAGS=@OCAMLLDFLAGS@
OCAMLTK_LIBDIR=@OCAMLTK_LIBDIR@
OCAMLDEFS=@OCAMLDEFS@

LIBS=@LIBS@ -lnums -lunix

CMA = nums.cma unix.cma
CMXA = $(CMA:.cma=.cmxa)

OCAMLCOMPILE = $(OCAMLC) $(OCAMLDEFS) $(OCAMLINCLUDES) $(OCAMLCFLAGS)
OCAMLOPTCOMPILE = $(OCAMLOPT) $(OCAMLDEFS) $(OCAMLINCLUDES) $(OCAMLOPTCFLAGS)
OCAMLLINK = $(OCAMLC) -custom $(OCAMLLDFLAGS) -o $@ $(CMA) $^ -cclib "${LIBS}"
OCAMLOPTLINK = $(OCAMLOPT) $(OCAMLLDFLAGS) -o $@ $(CMXA) $^ -cclib "${LIBS}"

.mli.cmi:
	$(OCAMLCOMPILE) -c -o $@ $< 

.ml.cmo:
	$(OCAMLCOMPILE) -c -o $@ $<

.ml.cmx:
	$(OCAMLOPTCOMPILE) -c -o $@ $<

.ml.p.cmx:
	$(OCAMLOPTCOMPILE) -p -c -o $@ $<

%.o: %.c
	$(OCAMLCOMPILE) -ccopt "$(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)" -c -o $@ $<

%.cma:
	$(OCAMLC) -a -o $@ $^

%.cmxa %.a:
	$(OCAMLOPT) -a -o $*.cmxa $^
	$(RANLIB) $*.a

maintainer-clean-local::
	rm -f *.cm* *.o *.a
	rm -f *~

depend: 
	(cd ${srcdir}; $(OCAMLDEP) *.mli *.ml > .depend)

include .depend

