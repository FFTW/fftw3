noinst_SCRIPTS = gen_notwiddle

EXTRA_DIST = .depend assignmentsToVfpinstrs.ml balanceVfpinstrs.ml	\
complex.ml expr.ml exprdag.ml fft.ml genUtil.ml gen_hc2hc.ml		\
gen_hc2real.ml gen_notwiddle.ml gen_notwiddle_fixedstride.ml		\
gen_real2hc.ml gen_realeven.ml gen_realeven2.ml gen_realodd.ml		\
gen_realodd2.ml gen_twiddle.ml id.ml k7Basics.ml k7ExecutionModel.ml	\
k7FlatInstructionScheduling.ml k7InstructionSchedulingBasics.ml		\
k7RegisterAllocationBasics.ml k7RegisterAllocator.ml			\
k7RegisterAllocatorEATranslation.ml k7RegisterAllocatorInit.ml		\
k7RegisterReallocation.ml k7Translate.ml k7Unparsing.ml			\
k7Vectorization.ml magic.ml memoMonad.ml nonDetMonad.ml			\
nullVectorization.ml number.ml stateMonad.ml symmetry.ml twiddle.ml	\
util.ml vAnnotatedScheduler.ml vDag.ml vFpBasics.ml vFpUnparsing.ml	\
vImproveSchedule.ml vK7Optimization.ml vScheduler.ml vSimdBasics.ml	\
vSimdIndexing.ml vSimdUnparsing.ml variable.ml				\
assignmentsToVfpinstrs.mli balanceVfpinstrs.mli complex.mli expr.mli	\
exprdag.mli fft.mli id.mli k7Basics.mli k7ExecutionModel.mli		\
k7FlatInstructionScheduling.mli k7InstructionSchedulingBasics.mli	\
k7RegisterAllocationBasics.mli k7RegisterAllocator.mli			\
k7RegisterAllocatorEATranslation.mli k7RegisterAllocatorInit.mli	\
k7RegisterReallocation.mli k7Translate.mli k7Unparsing.mli		\
k7Vectorization.mli memoMonad.mli nonDetMonad.mli			\
nullVectorization.mli number.mli stateMonad.mli util.mli		\
vAnnotatedScheduler.mli vDag.mli vFpBasics.mli vFpUnparsing.mli		\
vImproveSchedule.mli vK7Optimization.mli vScheduler.mli			\
vSimdBasics.mli vSimdIndexing.mli vSimdUnparsing.mli variable.mli

libgenfft_objects = magic.cmo util.cmo number.cmo variable.cmo		\
expr.cmo stateMonad.cmo memoMonad.cmo exprdag.cmo complex.cmo		\
symmetry.cmo twiddle.cmo fft.cmo nonDetMonad.cmo vFpBasics.cmo		\
balanceVfpinstrs.cmo vSimdBasics.cmo assignmentsToVfpinstrs.cmo		\
k7Basics.cmo id.cmo k7ExecutionModel.cmo				\
k7InstructionSchedulingBasics.cmo k7FlatInstructionScheduling.cmo	\
k7RegisterAllocationBasics.cmo vFpUnparsing.cmo vSimdUnparsing.cmo	\
k7Unparsing.cmo k7RegisterAllocatorEATranslation.cmo			\
k7RegisterAllocatorInit.cmo k7RegisterAllocator.cmo			\
k7RegisterReallocation.cmo k7Translate.cmo nullVectorization.cmo	\
k7Vectorization.cmo vDag.cmo vScheduler.cmo vAnnotatedScheduler.cmo	\
vImproveSchedule.cmo vSimdIndexing.cmo vK7Optimization.cmo genUtil.cmo

libgenfft.cma: $(libgenfft_objects)
	$(OCAMLC) -a -o $@ $^

libgenfft.cmxa libgenfft.a: $(libgenfft_objects:.cmo=.cmx)
	$(OCAMLOPT) -a -o libgenfft.cmxa $^
	$(RANLIB) libgenfft.a

gen_notwiddle_OBJECTS = gen_notwiddle.cmo
gen_notwiddle: libgenfft.cmxa $(gen_notwiddle_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
gen_notwiddle.nonopt: libgenfft.cma $(gen_notwiddle_OBJECTS)
	$(OCAMLLINK)
clean-local::
	rm -f gen_notwiddle gen_notw.nonopt

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

