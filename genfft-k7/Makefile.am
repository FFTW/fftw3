noinst_SCRIPTS = gen_notw gen_twiddle

EXTRA_DIST = .depend algsimp.ml assignmentsToVfpinstrs.ml		\
assoctable.ml balanceVfpinstrs.ml complex.ml expr.ml fft.ml genUtil.ml	\
gen_notw.ml gen_twiddle.ml id.ml k7Basics.ml k7ExecutionModel.ml	\
k7FlatInstructionScheduling.ml k7InstructionSchedulingBasics.ml		\
k7RegisterAllocationBasics.ml k7RegisterAllocator.ml			\
k7RegisterAllocatorEATranslation.ml k7RegisterAllocatorInit.ml		\
k7RegisterReallocation.ml k7Translate.ml k7Unparsing.ml			\
k7Vectorization.ml littlesimp.ml magic.ml memoMonad.ml monads.ml	\
nonDetMonad.ml nullVectorization.ml number.ml oracle.ml stateMonad.ml	\
to_alist.ml twiddle.ml util.ml vAnnotatedScheduler.ml vDag.ml		\
vFpBasics.ml vFpUnparsing.ml vImproveSchedule.ml vK7Optimization.ml	\
vScheduler.ml vSimdBasics.ml vSimdIndexing.ml vSimdUnparsing.ml		\
variable.ml algsimp.mli assignmentsToVfpinstrs.mli assoctable.mli	\
balanceVfpinstrs.mli complex.mli expr.mli fft.mli id.mli k7Basics.mli	\
k7ExecutionModel.mli k7FlatInstructionScheduling.mli			\
k7InstructionSchedulingBasics.mli k7RegisterAllocationBasics.mli	\
k7RegisterAllocator.mli k7RegisterAllocatorEATranslation.mli		\
k7RegisterAllocatorInit.mli k7RegisterReallocation.mli k7Translate.mli	\
k7Unparsing.mli k7Vectorization.mli littlesimp.mli memoMonad.mli	\
nonDetMonad.mli nullVectorization.mli number.mli oracle.mli		\
stateMonad.mli to_alist.mli twiddle.mli util.mli			\
vAnnotatedScheduler.mli vDag.mli vFpBasics.mli vFpUnparsing.mli		\
vImproveSchedule.mli vK7Optimization.mli vScheduler.mli			\
vSimdBasics.mli vSimdIndexing.mli vSimdUnparsing.mli variable.mli

libgenfft_objects = magic.cmo util.cmo number.cmo variable.cmo		\
expr.cmo stateMonad.cmo memoMonad.cmo monads.cmo littlesimp.cmo		\
assoctable.cmo oracle.cmo to_alist.cmo algsimp.cmo complex.cmo		\
twiddle.cmo fft.cmo nonDetMonad.cmo vFpBasics.cmo balanceVfpinstrs.cmo	\
vSimdBasics.cmo k7Basics.cmo id.cmo k7ExecutionModel.cmo		\
k7InstructionSchedulingBasics.cmo k7FlatInstructionScheduling.cmo	\
k7RegisterAllocationBasics.cmo vFpUnparsing.cmo				\
assignmentsToVfpinstrs.cmo vSimdUnparsing.cmo k7Unparsing.cmo		\
k7RegisterAllocatorEATranslation.cmo k7RegisterAllocatorInit.cmo	\
k7RegisterAllocator.cmo k7RegisterReallocation.cmo k7Translate.cmo	\
nullVectorization.cmo k7Vectorization.cmo vDag.cmo vScheduler.cmo	\
vAnnotatedScheduler.cmo vImproveSchedule.cmo vSimdIndexing.cmo		\
vK7Optimization.cmo genUtil.cmo

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
maintainer-clean-local::
	rm -f gen_notw gen_notw.nonopt

gen_twiddle_OBJECTS = gen_twiddle.cmo
gen_twiddle: libgenfft.cmxa $(gen_twiddle_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
gen_twiddle.nonopt: libgenfft.cma $(gen_twiddle_OBJECTS)
	$(OCAMLLINK)
maintainer-clean-local::
	rm -f gen_twiddle gen_twiddle.nonopt

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

