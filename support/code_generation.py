#!/usr/bin/env python3
"""Generate the complete FFTW source set from git checkout.

Usage: python3 support/code_generation.py -j 8

Requires Python 3.8+, OCaml/ocamlbuild/Num, GCC, Perl and Bash. GNU indent
is used when available. The existing OCaml and Fortran generators are reused.
"""

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile


# Each entry is (name, generator, arguments, sizes). Edit sizes here to
# customize the codelets; empty lists retain optional generator families.
# Order matters: codlist.c uses this order when registering solvers.
RECIPES = {
    "dft/scalar/codelets": [
        ("n1", "gen_notw",
         "-compact -variables 4 -pipeline-latency 4 -n {n} -name n1_{n} -include dft/scalar/n.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64, 20, 25]),
        ("t1", "gen_twiddle",
         "-compact -variables 4 -pipeline-latency 4 -n {n} -name t1_{n} -include dft/scalar/t.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64, 20, 25]),
        ("t2", "gen_twiddle",
         "-compact -variables 4 -pipeline-latency 4 -twiddle-log3 -precompute-twiddles -n {n} -name t2_{n} -include dft/scalar/t.h",
         [4, 8, 16, 32, 64, 5, 10, 20, 25]),
        ("f1", "gen_twiddle",
         "-compact -variables 4 -pipeline-latency 4 -dif -n {n} -name f1_{n} -include dft/scalar/f.h",
         []),
        ("f2", "gen_twiddle",
         "-compact -variables 4 -pipeline-latency 4 -twiddle-log3 -precompute-twiddles -dif -n {n} -name f2_{n} -include dft/scalar/f.h",
         []),
        # The q codelets grow quadratically; use small sizes (normally <= 8).
        ("q1", "gen_twidsq",
         "-compact -variables 4 -pipeline-latency 4 -reload-twiddle -dif -n {n} -name q1_{n} -include dft/scalar/q.h",
         [2, 4, 8, 3, 5, 6]),
        ("q2", "gen_twidsq",
         "-compact -variables 4 -pipeline-latency 4 -twiddle-log3 -precompute-twiddles -dif -n {n} -name q2_{n} -include dft/scalar/q.h",
         []),
    ],
    "rdft/scalar/r2cf": [
        ("r2cf", "gen_r2cf",
         "-compact -variables 4 -pipeline-latency 4 -n {n} -name r2cf_{n} -include rdft/scalar/r2cf.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64, 128, 20, 25]),
        ("hf", "gen_hc2hc",
         "-compact -variables 4 -pipeline-latency 4 -n {n} -dit -name hf_{n} -include rdft/scalar/hf.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64, 20, 25]),
        ("hf2", "gen_hc2hc",
         "-compact -variables 4 -pipeline-latency 4 -twiddle-log3 -precompute-twiddles -n {n} -dit -name hf2_{n} -include rdft/scalar/hf.h",
         [4, 8, 16, 32, 5, 20, 25]),
        ("r2cfII", "gen_r2cf",
         "-compact -variables 4 -pipeline-latency 4 -n {n} -name r2cfII_{n} -dft-II -include rdft/scalar/r2cfII.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64, 20, 25]),
        ("hc2cf", "gen_hc2c",
         "-compact -variables 4 -pipeline-latency 4 -n {n} -dit -name hc2cf_{n} -include rdft/scalar/hc2cf.h",
         [2, 4, 6, 8, 10, 12, 16, 32, 20]),
        ("hc2cf2", "gen_hc2c",
         "-compact -variables 4 -pipeline-latency 4 -twiddle-log3 -precompute-twiddles -n {n} -dit -name hc2cf2_{n} -include rdft/scalar/hc2cf.h",
         [4, 8, 16, 32, 20]),
        ("hc2cfdft", "gen_hc2cdft",
         "-compact -variables 4 -pipeline-latency 4 -n {n} -dit -name hc2cfdft_{n} -include rdft/scalar/hc2cf.h",
         [2, 4, 6, 8, 10, 12, 16, 32, 20]),
        ("hc2cfdft2", "gen_hc2cdft",
         "-compact -variables 4 -pipeline-latency 4 -twiddle-log3 -precompute-twiddles -n {n} -dit -name hc2cfdft2_{n} -include rdft/scalar/hc2cf.h",
         [4, 8, 16, 32, 20]),
    ],
    "rdft/scalar/r2cb": [
        ("r2cb", "gen_r2cb",
         "-compact -variables 4 -pipeline-latency 4 -sign 1 -n {n} -name r2cb_{n} -include rdft/scalar/r2cb.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64, 128, 20, 25]),
        ("hb", "gen_hc2hc",
         "-compact -variables 4 -pipeline-latency 4 -sign 1 -n {n} -dif -name hb_{n} -include rdft/scalar/hb.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64, 20, 25]),
        ("hb2", "gen_hc2hc",
         "-compact -variables 4 -pipeline-latency 4 -sign 1 -twiddle-log3 -precompute-twiddles -n {n} -dif -name hb2_{n} -include rdft/scalar/hb.h",
         [4, 8, 16, 32, 5, 20, 25]),
        ("r2cbIII", "gen_r2cb",
         "-compact -variables 4 -pipeline-latency 4 -sign 1 -n {n} -name r2cbIII_{n} -dft-III -include rdft/scalar/r2cbIII.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64, 20, 25]),
        ("hc2cb", "gen_hc2c",
         "-compact -variables 4 -pipeline-latency 4 -sign 1 -n {n} -dif -name hc2cb_{n} -include rdft/scalar/hc2cb.h",
         [2, 4, 6, 8, 10, 12, 16, 32, 20]),
        ("hc2cb2", "gen_hc2c",
         "-compact -variables 4 -pipeline-latency 4 -sign 1 -twiddle-log3 -precompute-twiddles -n {n} -dif -name hc2cb2_{n} -include rdft/scalar/hc2cb.h",
         [4, 8, 16, 32, 20]),
        ("hc2cbdft", "gen_hc2cdft",
         "-compact -variables 4 -pipeline-latency 4 -sign 1 -n {n} -dif -name hc2cbdft_{n} -include rdft/scalar/hc2cb.h",
         [2, 4, 6, 8, 10, 12, 16, 32, 20]),
        ("hc2cbdft2", "gen_hc2cdft",
         "-compact -variables 4 -pipeline-latency 4 -sign 1 -n {n} -dif -name hc2cbdft2_{n} -include rdft/scalar/hc2cb.h",
         [4, 8, 16, 32, 20]),
    ],
    "rdft/scalar/r2r": [
        ("e00", "gen_r2r",
         "-compact -variables 4 -pipeline-latency 4 -redft00 -n {n} -name e00_{n} -include rdft/scalar/r2r.h",
         []),
        ("e01", "gen_r2r",
         "-compact -variables 4 -pipeline-latency 4 -redft01 -n {n} -name e01_{n} -include rdft/scalar/r2r.h",
         [8]),
        ("e10", "gen_r2r",
         "-compact -variables 4 -pipeline-latency 4 -redft10 -n {n} -name e10_{n} -include rdft/scalar/r2r.h",
         [8]),
        ("e11", "gen_r2r",
         "-compact -variables 4 -pipeline-latency 4 -redft11 -n {n} -name e11_{n} -include rdft/scalar/r2r.h",
         []),
        ("o00", "gen_r2r",
         "-compact -variables 4 -pipeline-latency 4 -rodft00 -n {n} -name o00_{n} -include rdft/scalar/r2r.h",
         []),
        ("o01", "gen_r2r",
         "-compact -variables 4 -pipeline-latency 4 -rodft01 -n {n} -name o01_{n} -include rdft/scalar/r2r.h",
         []),
        ("o10", "gen_r2r",
         "-compact -variables 4 -pipeline-latency 4 -rodft10 -n {n} -name o10_{n} -include rdft/scalar/r2r.h",
         []),
        ("o11", "gen_r2r",
         "-compact -variables 4 -pipeline-latency 4 -rodft11 -n {n} -name o11_{n} -include rdft/scalar/r2r.h",
         []),
        ("dht", "gen_r2r",
         "-compact -variables 4 -pipeline-latency 4 -dht -sign 1 -n {n} -name dht_{n} -include rdft/scalar/r2r.h",
         []),
    ],
    "dft/simd/common": [
        ("n1fv", "gen_notw_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -name n1fv_{n} -include dft/simd/n1f.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64, 128, 20, 25]),
        ("n1bv", "gen_notw_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -sign 1 -n {n} -name n1bv_{n} -include dft/simd/n1b.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64, 128, 20, 25]),
        ("n2fv", "gen_notw_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -name n2fv_{n} -with-ostride 2 -include dft/simd/n2f.h -store-multiple 2",
         [2, 4, 6, 8, 10, 12, 14, 16, 32, 64, 20]),
        ("n2bv", "gen_notw_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -sign 1 -n {n} -name n2bv_{n} -with-ostride 2 -include dft/simd/n2b.h -store-multiple 2",
         [2, 4, 6, 8, 10, 12, 14, 16, 32, 64, 20]),
        ("n2sv", "gen_notw",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -name n2sv_{n} -with-ostride 1 -include dft/simd/n2s.h -store-multiple 4",
         [4, 8, 16, 32, 64]),
        ("t1fuv", "gen_twiddle_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -name t1fuv_{n} -include dft/simd/t1fu.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10]),
        ("t1fv", "gen_twiddle_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -name t1fv_{n} -include dft/simd/t1f.h",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64, 20, 25]),
        ("t2fv", "gen_twiddle_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -name t2fv_{n} -include dft/simd/t2f.h",
         [2, 4, 8, 16, 32, 64, 5, 10, 20, 25]),
        ("t3fv", "gen_twiddle_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -twiddle-log3 -precompute-twiddles -no-generate-bytw -n {n} -name t3fv_{n} -include dft/simd/t3f.h",
         [4, 8, 16, 32, 5, 10, 20, 25]),
        ("t1buv", "gen_twiddle_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -name t1buv_{n} -include dft/simd/t1bu.h -sign 1",
         [2, 3, 4, 5, 6, 7, 8, 9, 10]),
        ("t1bv", "gen_twiddle_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -name t1bv_{n} -include dft/simd/t1b.h -sign 1",
         [2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 32, 64, 20, 25]),
        ("t2bv", "gen_twiddle_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -name t2bv_{n} -include dft/simd/t2b.h -sign 1",
         [2, 4, 8, 16, 32, 64, 5, 10, 20, 25]),
        ("t3bv", "gen_twiddle_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -twiddle-log3 -precompute-twiddles -no-generate-bytw -n {n} -name t3bv_{n} -include dft/simd/t3b.h -sign 1",
         [4, 8, 16, 32, 5, 10, 20, 25]),
        ("t1sv", "gen_twiddle",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -name t1sv_{n} -include dft/simd/ts.h",
         [2, 4, 8, 16, 32]),
        ("t2sv", "gen_twiddle",
         "-simd -compact -variables 4 -pipeline-latency 8 -twiddle-log3 -precompute-twiddles -n {n} -name t2sv_{n} -include dft/simd/ts.h",
         [4, 8, 16, 32]),
        ("q1fv", "gen_twidsq_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -dif -name q1fv_{n} -include dft/simd/q1f.h",
         [2, 4, 5, 8]),
        ("q1bv", "gen_twidsq_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -n {n} -dif -name q1bv_{n} -include dft/simd/q1b.h -sign 1",
         [2, 4, 5, 8]),
    ],
    "rdft/simd/common": [
        ("hc2cfdftv", "gen_hc2cdft_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -trivial-stores -variables 32 -no-generate-bytw -n {n} -dit -name hc2cfdftv_{n} -include rdft/simd/hc2cfv.h",
         [2, 4, 6, 8, 10, 12, 16, 32, 20]),
        ("hc2cbdftv", "gen_hc2cdft_c",
         "-simd -compact -variables 4 -pipeline-latency 8 -trivial-stores -variables 32 -no-generate-bytw -n {n} -dif -sign 1 -name hc2cbdftv_{n} -include rdft/simd/hc2cbv.h",
         [2, 4, 6, 8, 10, 12, 16, 32, 20]),
    ],
}

# Solver table symbol and Automake source-list variable for each group.
GROUPS = {
    "dft/scalar/codelets": ("solvtab_dft_standard", "DFT_SCALAR_CODELETS"),
    "rdft/scalar/r2cf": ("solvtab_rdft_r2cf", "RDFT_R2CF_CODELETS"),
    "rdft/scalar/r2cb": ("solvtab_rdft_r2cb", "RDFT_R2CB_CODELETS"),
    "rdft/scalar/r2r": ("solvtab_rdft_r2r", "RDFT_R2R_CODELETS"),
    "dft/simd/common": ("solvtab_dft", "DFT_SIMD_CODELETS"),
    "rdft/simd/common": ("solvtab_rdft", "RDFT_SIMD_CODELETS"),
}

SIMD_HEADERS = {
    "sse2": "simd-sse2.h",
    "avx": "simd-avx.h",
    "avx-128-fma": "simd-avx-128-fma.h",
    "avx2": "simd-avx2.h",
    "avx2-128": "simd-avx2-128.h",
    "avx512": "simd-avx512.h",
    "kcvi": "simd-kcvi.h",
    "altivec": "simd-altivec.h",
    "vsx": "simd-vsx.h",
    "neon": "simd-neon.h",
    "sve128": "simd-maskedsve128.h",
    "sve256": "simd-maskedsve256.h",
    "sve512": "simd-maskedsve512.h",
    "sve1024": "simd-maskedsve1024.h",
    "sve2048": "simd-maskedsve2048.h",
    "lsx": "simd-lsx.h",
    "lasx": "simd-lasx.h",
    "generic-simd128": "simd-generic128.h",
    "generic-simd256": "simd-generic256.h",
}

INTERFACES = (
    "api/fftw3.f", "api/fftw3.f03.in", "api/fftw3l.f03", "api/fftw3q.f03",
    "mpi/fftw3-mpi.f03.in", "mpi/fftw3l-mpi.f03.in", "mpi/f03-wrap.c",
)
INDENT_FLAGS = "-kr -cs -i5 -l800 -fca -nfc1 -sc -sob -cli4 -TR -Tplanner -TV"


def run(command, cwd, text=None):
    result = subprocess.run(command, cwd=cwd, input=text, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode:
        raise RuntimeError("{} (exit {})\n{}".format(
            " ".join(map(str, command)), result.returncode,
            result.stderr.strip() or result.stdout.strip()))
    return result.stdout


def write_file(path, text):
    """Publish a complete file; preserve timestamps when content is unchanged."""
    data = text.encode("utf-8")
    if path.exists() and path.read_bytes() == data:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=path.parent, prefix="." + path.name,
                                     delete=False) as stream:
        temporary = Path(stream.name)
        try:
            stream.write(data)
            stream.close()
            temporary.chmod(0o644)
            temporary.replace(path)
        finally:
            temporary.unlink(missing_ok=True)


def codelets():
    for directory, recipes in RECIPES.items():
        for name, generator, arguments, sizes in recipes:
            for size in sizes:
                yield (Path(directory) / (name + "_" + str(size) + ".c"),
                       generator, arguments.format(n=size).split())


def expected_files():
    paths = [path for path, _, _ in codelets()]
    paths.extend(Path(directory) / "codlist.c" for directory in RECIPES)
    for kind in ("dft", "rdft"):
        common = [path.name for path, _, _ in codelets()
                  if path.parent == Path(kind) / "simd/common"]
        for isa in SIMD_HEADERS:
            paths.extend(Path(kind) / "simd" / isa / name
                         for name in common + ["codlist.c", "genus.c"])
    paths.extend(map(Path, INTERFACES))
    paths.append(Path("support/codelets.mk"))
    return paths


def check_files(root):
    expected = set(expected_files())
    missing = [str(path) for path in sorted(expected)
               if not (root / path).is_file() or not (root / path).stat().st_size]
    # CMake uses globs. Reject stale codelets instead of silently compiling
    # a removed codelet, or deleting a developer's custom generated source.
    extras = []
    for directory in {path.parent for path in expected if path.suffix == ".c"}:
        if str(directory) == "mpi":
            continue
        for path in (root / directory).glob("*.c"):
            relative = path.relative_to(root)
            if relative not in expected and path.name != "genus.c":
                extras.append(str(relative))
    if missing or extras:
        raise RuntimeError("\n".join(
            (["Missing/empty generated files:"] + missing if missing else [])
            + (["Unexpected codelets; update RECIPES or remove stale files:"]
               + sorted(extras) if extras else [])))
    print("Complete: {} generated files (all 19 ISA directories per transform)."
          .format(len(expected)), flush=True)


def build_generators(root, jobs):
    generators = sorted({entry[1] for recipes in RECIPES.values() for entry in recipes}
                        | {"gen_mdct"})
    print("Building {} OCaml generators...".format(len(generators)), flush=True)
    run(["ocamlbuild", "-classic-display", "-j", str(jobs), "-pkgs", "num"]
        + [generator + ".native" for generator in generators], root / "genfft")


def generate_codelets(root, jobs, force, indent):
    dependencies = [Path(__file__), root / "COPYRIGHT"]
    dependencies += list((root / "genfft").glob("*.ml"))
    dependencies += list((root / "genfft").glob("*.mli"))
    dependencies += list((root / "support").glob("codelet_prelude.*"))
    changed = max(path.stat().st_mtime_ns for path in dependencies)
    copyright_text = (root / "COPYRIGHT").read_text()
    preludes = {kind: (root / "support" / ("codelet_prelude." + kind)).read_text()
                .replace("/* Generated on @DATE@ */\n", "")
                for kind in ("dft", "rdft")}

    def generate(job):
        relative, generator, arguments = job
        target = root / relative
        executable = root / "genfft" / (generator + ".native")
        if (not force and target.is_file() and target.stat().st_size
                and target.stat().st_mtime_ns >= max(
                    changed, executable.stat().st_mtime_ns)):
            return False
        # Use the original relative command spelling in the generated comment.
        command = [os.path.relpath(executable, target.parent)]
        text = copyright_text + preludes[relative.parts[0]]
        text += "#if defined(ARCH_PREFERS_FMA) || defined(ISA_EXTENSION_PREFERS_FMA)\n\n"
        text += run(command + ["-fma"] + arguments, target.parent)
        text += "\n#else\n\n"
        text += run(command + arguments, target.parent)
        text += "\n#endif\n"
        if text.count("/* Generated by:") != 2:
            raise RuntimeError("Incomplete generator output for " + str(relative))
        if indent:
            text = run([indent] + INDENT_FLAGS.split(), target.parent, text)
        write_file(target, text)
        return True

    tasks = list(codelets())
    print("Generating {} scalar/SIMD codelets...".format(len(tasks)), flush=True)
    failures, completed, rebuilt = [], 0, 0
    with ThreadPoolExecutor(max_workers=jobs) as pool:
        futures = {pool.submit(generate, task): task[0] for task in tasks}
        for future in as_completed(futures):
            path = futures[future]
            try:
                rebuilt += future.result()
            except Exception as error:
                failures.append(str(path))
                print("FAIL {}: {}".format(path, error), file=sys.stderr, flush=True)
            completed += 1
            if completed % 25 == 0 or completed == len(tasks):
                print("  {}/{} finished".format(completed, len(tasks)), flush=True)
    if failures:
        raise RuntimeError("Code generation failed for: " + ", ".join(failures))
    print("  {} regenerated, {} up to date".format(rebuilt, len(tasks) - rebuilt),
          flush=True)


def generate_tables_and_wrappers(root):
    manifest = "# Generated automatically. Edit support/code_generation.py instead.\n"
    for directory, (solvtab, variable) in GROUPS.items():
        names = [path.stem for path, _, _ in codelets()
                 if path.parent == Path(directory)]
        manifest += "\n" + variable + " ="
        for name in names:
            manifest += " \\\n    " + name + ".c"
        manifest += "\n"
        simd = directory.endswith("/simd/common")
        rename = "XSIMD" if simd else "X"
        text = '#include "kernel/ifftw.h"\n'
        text += "#include SIMD_HEADER\n\n" if simd else "\n\n"
        text += "".join("extern void {}(codelet_{})(planner *);\n".format(rename, n)
                        for n in names)
        text += "\n\nextern const solvtab {}({});\n".format(rename, solvtab)
        text += "const solvtab {}({}) = {{\n".format(rename, solvtab)
        text += "".join("   SOLVTAB({}(codelet_{})),\n".format(rename, n)
                        for n in names)
        text += "   SOLVTAB_END\n};\n"
        write_file(root / directory / "codlist.c", text)
        if simd:
            for isa, header in SIMD_HEADERS.items():
                for name in names + ["codlist", "genus"]:
                    text = ('/* Generated automatically.  DO NOT EDIT! */\n'
                            '#define SIMD_HEADER "simd-support/{}"\n'
                            '#include "../common/{}.c"\n').format(header, name)
                    write_file(root / Path(directory).parent / isa / (name + ".c"), text)
    write_file(root / "support/codelets.mk", manifest)


def generate_interfaces(root):
    print("Generating all serial and MPI Fortran interfaces...", flush=True)
    # Reuse the existing header-to-Fortran generators, but make every command
    # and every pipeline fail reliably. They do not include an MPI installation.
    def shell(directory, script, *arguments):
        return run(["bash", "-e", "-o", "pipefail", script] + list(arguments),
                   root / directory)

    header = (root / "api/fftw3.h").read_text()
    constants = re.findall(r"([A-Z0-9_]+)=([+-]?[0-9]+)", header)
    constants += re.findall(r"#define +([A-Z0-9_]+) +\(([+-]?[0-9]+)U?\)", header)
    constants += [(name, str(int(value) << int(shift))) for name, value, shift in
                  re.findall(r"#define +([A-Z0-9_]+) +\(([0-9]+)U? *<< *([0-9]+)\)", header)]
    write_file(root / "api/fftw3.f", "".join(
        "      INTEGER {}\n      PARAMETER ({}={})\n".format(name, name, value)
        for name, value in constants))

    for directory, name, precisions in (
            ("api", "fftw3.f03.in", ("d", "f")),
            ("api", "fftw3l.f03", ("l",)),
            ("api", "fftw3q.f03", ("q",)),
            ("mpi", "fftw3-mpi.f03.in", ("d", "f")),
            ("mpi", "fftw3l-mpi.f03.in", ("l",))):
        text = shell(directory, "f03api.sh", *precisions)
        for precision in precisions:
            suffix = "" if precision == "d" else precision
            function = "mpi_init" if directory == "mpi" else "plan_dft"
            if "fftw{}_{}".format(suffix, function) not in text:
                raise RuntimeError("Missing {} interface in {}".format(precision, name))
        if len(precisions) == 1:
            text = "".join(line for line in text.splitlines(keepends=True)
                           if "parameter" not in line)
        if name == "fftw3l-mpi.f03.in":
            text = text.replace("fftw3.f03", "fftw3l.f03")
        write_file(root / directory / name, text)
    text = shell("mpi", "f03-wrap.sh")
    if "MPI_Comm_f2c" not in text:
        raise RuntimeError("Missing MPI Fortran C wrappers")
    write_file(root / "mpi/f03-wrap.c", text)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count() or 1,
                        help="maximum parallel jobs (default: CPU count)")
    parser.add_argument("--source-dir", type=Path, default=Path(__file__).resolve().parent.parent,
                        help="FFTW source tree (default: parent of this script's directory)")
    parser.add_argument("--force", action="store_true", help="regenerate every codelet")
    parser.add_argument("--no-indent", action="store_true", help="skip optional GNU indent formatting")
    parser.add_argument("--check", action="store_true", help="check generated-file coverage without running generators")
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    root = args.source_dir.resolve()
    if not (root / "genfft/gen_notw.ml").is_file():
        parser.error("--source-dir must name an FFTW source tree")
    if args.check:
        check_files(root)
        return
    for command in ("ocamlbuild", "ocamlfind", "ocamlopt", "gcc", "perl", "bash"):
        if not shutil.which(command):
            raise RuntimeError("Required program not found: " + command)
    run(["ocamlfind", "query", "num"], root)
    indent = None if args.no_indent else shutil.which("indent")
    if indent:
        # Some GNU indent versions print their version but return EX_USAGE.
        version = subprocess.run([indent, "--version"], text=True,
                                 stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if "GNU indent" not in version.stdout:
            raise RuntimeError("GNU indent is required for formatting; use --no-indent to skip it")
    if not indent and not args.no_indent:
        print("GNU indent not found; generating unformatted codelets.", file=sys.stderr)
    build_generators(root, args.jobs)
    # Interface generation is cheap: detect missing header-generation tools
    # before starting the expensive codelets.
    generate_interfaces(root)
    generate_codelets(root, args.jobs, args.force, indent)
    generate_tables_and_wrappers(root)
    check_files(root)


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError) as error:
        sys.exit("code_generation.py: " + str(error))
