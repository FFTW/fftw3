noinst_SCRIPTS = gen_notw gen_twiddle

EXTRA_DIST = .depend algsimp.ml algsimp.mli assignmentsToVfpinstrs.ml	\
assignmentsToVfpinstrs.mli assoctable.ml assoctable.mli			\
balanceVfpinstrs.ml balanceVfpinstrs.mli complex.ml complex.mli		\
expr.ml expr.mli fft.ml fft.mli genUtil.ml gen_notw.ml id.ml id.mli	\
k7Basics.ml k7Basics.mli k7ExecutionModel.ml k7ExecutionModel.mli	\
k7FlatInstructionScheduling.ml k7FlatInstructionScheduling.mli		\
k7InstructionSchedulingBasics.ml k7InstructionSchedulingBasics.mli	\
k7RegisterAllocationBasics.ml k7RegisterAllocationBasics.mli		\
k7RegisterAllocator.ml k7RegisterAllocator.mli				\
k7RegisterAllocatorEATranslation.ml					\
k7RegisterAllocatorEATranslation.mli k7RegisterAllocatorInit.ml		\
k7RegisterAllocatorInit.mli k7RegisterReallocation.ml			\
k7RegisterReallocation.mli k7Translate.ml k7Translate.mli		\
k7Unparsing.ml k7Unparsing.mli k7Vectorization.ml k7Vectorization.mli	\
littlesimp.ml littlesimp.mli magic.ml memoMonad.ml memoMonad.mli	\
monads.ml nonDetMonad.ml nonDetMonad.mli nullVectorization.ml		\
nullVectorization.mli number.ml number.mli oracle.ml oracle.mli		\
stateMonad.ml stateMonad.mli to_alist.ml to_alist.mli twiddle.ml	\
twiddle.mli util.ml util.mli vAnnotatedScheduler.ml			\
vAnnotatedScheduler.mli vDag.ml vDag.mli vFpBasics.ml vFpBasics.mli	\
vFpUnparsing.ml vFpUnparsing.mli vImproveSchedule.ml			\
vImproveSchedule.mli vK7Optimization.ml vK7Optimization.mli		\
vScheduler.ml vScheduler.mli vSimdBasics.ml vSimdBasics.mli		\
vSimdIndexing.ml vSimdIndexing.mli vSimdUnparsing.ml			\
vSimdUnparsing.mli variable.ml variable.mli

libgenfft_objects = magic.cmo util.cmo number.cmo variable.cmo		\
expr.cmo stateMonad.cmo memoMonad.cmo monads.cmo littlesimp.cmo		\
assoctable.cmo oracle.cmo to_alist.cmo algsimp.cmo complex.cmo		\
twiddle.cmo fft.cmo nonDetMonad.cmo vFpBasics.cmo balanceVfpinstrs.cmo	\
vSimdBasics.cmo assignmentsToVfpinstrs.cmo k7Basics.cmo id.cmo		\
k7ExecutionModel.cmo k7InstructionSchedulingBasics.cmo			\
k7FlatInstructionScheduling.cmo k7RegisterAllocationBasics.cmo		\
vFpUnparsing.cmo vSimdUnparsing.cmo k7Unparsing.cmo			\
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
clean-local::
	rm -f gen_notw gen_notw.nonopt

gen_twiddle_OBJECTS = gen_twiddle.cmo
gen_twiddle: libgenfft.cmxa $(gen_twiddle_OBJECTS:.cmo=.cmx)
	$(OCAMLOPTLINK)
gen_twiddle.nonopt: libgenfft.cma $(gen_twiddle_OBJECTS)
	$(OCAMLLINK)
clean-local::
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

clean-local::
	rm -f *.cm* *.o *.a
	rm -f *~

depend: 
	(cd ${srcdir}; $(OCAMLDEP) *.mli *.ml > .depend)

include .depend

