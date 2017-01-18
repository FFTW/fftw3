# Find OCaml installation

include( FindPackageHandleStandardArgs )

# find base ocaml binary
find_program( OCaml_OCAML_EXECUTABLE
  NAMES ocaml
)

# get the directory, used for hints for the rest of the binaries
if( OCaml_OCAML_EXECUTABLE )
    get_filename_component( OCaml_ROOT_DIR ${OCaml_OCAML_EXECUTABLE} PATH )
endif()

find_program( OCaml_OCAMLC_EXECUTABLE
  NAMES ocamlc
  HINTS ${OCaml_ROOT_DIR}
)

find_program( OCaml_OCAMLCOPT_EXECUTABLE
  NAMES ocamlc.opt
  HINTS ${OCaml_ROOT_DIR}
)

find_program( OCaml_OCAMLOPT_EXECUTABLE
  NAMES ocamlopt
  HINTS ${OCaml_ROOT_DIR}
)

find_program( OCaml_OCAMLOPTOPT_EXECUTABLE
  NAMES ocamlopt.opt
  HINTS ${OCaml_ROOT_DIR}
)

find_program( OCaml_OCAMLDEP_EXECUTABLE
  NAMES ocamldep
  HINTS ${OCaml_ROOT_DIR}
)

find_program( OCaml_OCAMLDEPOPT_EXECUTABLE
  NAMES ocamldep.opt
  HINTS ${OCaml_ROOT_DIR}
)

find_program( OCaml_OCAMLBUILDNATIVE_EXECUTABLE
  NAMES ocamlbuild.native ocamlbuild.byte
)


if( OCaml_OCAMLC_EXECUTABLE )
  execute_process(
    COMMAND ${OCaml_OCAMLC_EXECUTABLE} -version
    OUTPUT_VARIABLE OCaml_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  execute_process(
    COMMAND ${OCaml_OCAMLC_EXECUTABLE} -where
    OUTPUT_VARIABLE OCaml_LIBRARY_DIRS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

find_package_handle_standard_args( OCaml
    REQUIRED_VARS
    OCaml_OCAML_EXECUTABLE
    OCaml_OCAMLC_EXECUTABLE
    OCaml_OCAMLOPTOPT_EXECUTABLE
    OCaml_OCAMLDEPOPT_EXECUTABLE
    OCaml_OCAMLOPTOPT_EXECUTABLE
    OCaml_LIBRARY_DIRS
    OCaml_OCAMLBUILDNATIVE_EXECUTABLE
    VERSION_VAR OCaml_VERSION)
