# this makefile requires GNU make.

EXTRA_DIST = .depend algsimp.ml algsimp.mli annotate.ml annotate.mli	\
assoctable.ml assoctable.mli c.ml c.mli complex.ml complex.mli dag.ml	\
dag.mli expr.ml expr.mli fft.ml fft.mli conv.ml conv.mli gen_notw.ml	\
gen_twiddle.ml genutil.ml littlesimp.ml littlesimp.mli magic.ml		\
monads.ml number.ml number.mli oracle.ml oracle.mli schedule.ml		\
schedule.mli to_alist.ml to_alist.mli twiddle.ml twiddle.mli unique.ml	\
unique.mli util.ml util.mli variable.ml variable.mli gen_twidsq.ml	\
trig.ml trig.mli gen_trig.ml gen_athnotw.ml gen_athtw.ml

noinst_SCRIPTS=gen_notw gen_twiddle gen_twidsq gen_trig gen_athnotw	\
gen_athtw gen_conv

libgenfft_objects = unique.cmo magic.cmo util.cmo number.cmo	\
variable.cmo expr.cmo littlesimp.cmo monads.cmo complex.cmo	\
assoctable.cmo oracle.cmo algsimp.cmo to_alist.cmo dag.cmo	\
schedule.cmo annotate.cmo twiddle.cmo fft.cmo conv.cmo c.cmo	\
genutil.cmo trig.cmo

libgenfft.cma: $(libgenfft_objects)
	$(OCAMLC) -a -o $@ $^

libgenfft.cmxa libgenfft.a: $(libgenfft_objects:.cmo=.cmx)
	$(OCAMLOPT) -a -o libgenfft.cmxa $^
	$(RANLIB) libgenfft.a

gen_notw_OBJECTS = gen_notw.cmo
gen_notw: libgenfft.cmxa $(gen_notw_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
gen_notw.nonopt: libgenfft.cma $(gen_notw_OBJECTS)
	$(OCAMLLINK)
clean-local::
	rm -f gen_notw gen_notw.nonopt

gen_twiddle_OBJECTS = gen_twiddle.cmo
gen_twiddle: libgenfft.cmxa $(gen_twiddle_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
gen_twiddle.nonopt: libgenfft.cma $(gen_twiddle_OBJECTS)
	$(OCAMLLINK)
clean-local::
	rm -f gen_twiddle gen_twiddle.nonopt

gen_twidsq_OBJECTS = gen_twidsq.cmo
gen_twidsq: libgenfft.cmxa $(gen_twidsq_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
gen_twidsq.nonopt: libgenfft.cma $(gen_twidsq_OBJECTS)
	$(OCAMLLINK)
clean-local::
	rm -f gen_twidsq gen_twidsq.nonopt

gen_trig_OBJECTS = gen_trig.cmo
gen_trig: libgenfft.cmxa $(gen_trig_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
gen_trig.nonopt: libgenfft.cma $(gen_trig_OBJECTS)
	$(OCAMLLINK)
clean-local::
	rm -f gen_trig gen_trig.nonopt

gen_athnotw_OBJECTS = gen_athnotw.cmo
gen_athnotw: libgenfft.cmxa $(gen_athnotw_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
gen_athnotw.nonopt: libgenfft.cma $(gen_athnotw_OBJECTS)
	$(OCAMLLINK)
clean-local::
	rm -f gen_athnotw gen_athnotw.nonopt

gen_athtw_OBJECTS = gen_athtw.cmo
gen_athtw: libgenfft.cmxa $(gen_athtw_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
gen_athtw.nonopt: libgenfft.cma $(gen_athtw_OBJECTS)
	$(OCAMLLINK)
clean-local::
	rm -f gen_athtw gen_athtw.nonopt

gen_conv_OBJECTS = gen_conv.cmo
gen_conv: libgenfft.cmxa $(gen_conv_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
gen_conv.nonopt: libgenfft.cma $(gen_conv_OBJECTS)
	$(OCAMLLINK)
clean-local::
	rm -f gen_conv gen_conv.nonopt


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

clean-local::
	rm -f *.cm* *.o *.a
	rm -f *~

depend: 
	(cd ${srcdir}; $(OCAMLDEP) *.mli *.ml > .depend)

include .depend

