noinst_SCRIPTS = genfft

EXTRA_DIST = .depend assignmentsToVfpinstrs.ml balanceVfpinstrs.ml	\
codeletMisc.ml complex.ml expr.ml exprdag.ml fft.ml genUtil.ml		\
gen_hc2hc.ml gen_hc2real.ml gen_notwiddle.ml				\
gen_notwiddle_fixedstride.ml gen_real2hc.ml gen_realeven.ml		\
gen_realeven2.ml gen_realodd.ml gen_realodd2.ml gen_twiddle.ml		\
genfft.ml id.ml k7Basics.ml k7ExecutionModel.ml				\
k7FlatInstructionScheduling.ml k7InstructionSchedulingBasics.ml		\
k7RegisterAllocationBasics.ml k7RegisterAllocator.ml			\
k7RegisterAllocatorEATranslation.ml k7RegisterAllocatorInit.ml		\
k7RegisterReallocation.ml k7Translate.ml k7Unparsing.ml			\
k7Vectorization.ml magic.ml memoMonad.ml nonDetMonad.ml			\
nullVectorization.ml number.ml stateMonad.ml symmetry.ml twiddle.ml	\
util.ml vAnnotatedScheduler.ml vDag.ml vFpBasics.ml vFpUnparsing.ml	\
vImproveSchedule.ml vK7Optimization.ml vScheduler.ml vSimdBasics.ml	\
vSimdIndexing.ml vSimdUnparsing.ml variable.ml				\
assignmentsToVfpinstrs.mli balanceVfpinstrs.mli codeletMisc.mli		\
complex.mli expr.mli exprdag.mli fft.mli genUtil.mli gen_hc2hc.mli	\
gen_hc2real.mli gen_notwiddle.mli gen_real2hc.mli gen_realeven.mli	\
gen_realeven2.mli gen_realodd.mli gen_realodd2.mli gen_twiddle.mli	\
id.mli k7Basics.mli k7ExecutionModel.mli				\
k7FlatInstructionScheduling.mli k7InstructionSchedulingBasics.mli	\
k7RegisterAllocationBasics.mli k7RegisterAllocator.mli			\
k7RegisterAllocatorEATranslation.mli k7RegisterAllocatorInit.mli	\
k7RegisterReallocation.mli k7Translate.mli k7Unparsing.mli		\
k7Vectorization.mli magic.mli memoMonad.mli nonDetMonad.mli		\
nullVectorization.mli number.mli stateMonad.mli util.mli		\
vAnnotatedScheduler.mli vDag.mli vFpBasics.mli vFpUnparsing.mli		\
vImproveSchedule.mli vK7Optimization.mli vScheduler.mli			\
vSimdBasics.mli vSimdIndexing.mli vSimdUnparsing.mli variable.mli


genfft_OBJECTS = magic.cmo util.cmo number.cmo variable.cmo expr.cmo	\
stateMonad.cmo memoMonad.cmo exprdag.cmo complex.cmo symmetry.cmo	\
twiddle.cmo fft.cmo nonDetMonad.cmo vFpBasics.cmo balanceVfpinstrs.cmo	\
vSimdBasics.cmo assignmentsToVfpinstrs.cmo codeletMisc.cmo		\
k7Basics.cmo id.cmo k7ExecutionModel.cmo				\
k7InstructionSchedulingBasics.cmo k7FlatInstructionScheduling.cmo	\
k7RegisterAllocationBasics.cmo vFpUnparsing.cmo vSimdUnparsing.cmo	\
k7Unparsing.cmo k7RegisterAllocatorEATranslation.cmo			\
k7RegisterAllocatorInit.cmo k7RegisterAllocator.cmo			\
k7RegisterReallocation.cmo k7Translate.cmo nullVectorization.cmo	\
k7Vectorization.cmo vDag.cmo vScheduler.cmo vAnnotatedScheduler.cmo	\
vImproveSchedule.cmo vSimdIndexing.cmo vK7Optimization.cmo genUtil.cmo	\
gen_hc2hc.cmo gen_hc2real.cmo gen_notwiddle.cmo				\
gen_notwiddle_fixedstride.cmo gen_real2hc.cmo gen_realeven.cmo		\
gen_realeven2.cmo gen_realodd.cmo gen_realodd2.cmo gen_twiddle.cmo	\
genfft.cmo

genfft: $(genfft_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)

clean-local::
	rm -f genfft

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

