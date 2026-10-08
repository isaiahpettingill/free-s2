# free-s2

A small, independent implementation of the **1988 New S language**, written
in C89 for Linux and 8086 real-mode DOS. Open Watcom builds the DOS version;
optional fixed-form Fortran 77 routines use GNU Fortran on Linux and
Microsoft FORTRAN 5 on DOS.

Use it to explore data, write vector-based programs, fit small linear
regressions, and export charts. This is a useful, bounded reconstruction,
not a complete historical S distribution. No Bell Labs source is used.

## Get started

```sh
make test
./build/s2 -e 'mean(1:10)'
./build/s2
```

At the prompt:

```s
x <- c(2,4,6,8)
x[x > 4]
center <- function(x) x - mean(x)
center(x)
q()
```

Run the included data analysis and produce SVG/PostScript charts:

```sh
cd examples
../build/s2 ANALYZE.S
```

This reads `DATA.TXT`, calculates grouped means, fits a straight line, writes
`FIT.SVG`, `FIT.PS`, `RESID.TXT`, and saves the data and fit in `WORK.S`.

- [Language and usage guide](docs/guide.md): build, syntax, data, functions, statistics, charts, saving work.
- [Feature set](docs/features.md): supported functions and practical limits.
- [Compatibility matrix](docs/compatibility.md): manual evidence, historical ambiguities, deliberate differences.
- [Validation](docs/validation.md): native and DOS test results and reproduction.

## Other builds

```sh
make test-fortran
make dos-local
```

`test-fortran` needs GCC-compatible C and GNU Fortran. `dos-local` needs an
Open Watcom installation with `WATCOM`, `INCLUDE`, and its compiler directory
on `PATH`. It builds `build/S2.EXE` with `-0 -ml`, without an 8087 requirement.

The sibling [dosbox-agent-tools](https://github.com/isaiahpettingill/dosbox-agent-tools)
repository provides compiler containers and the PowerShell DOSBox test
module. With its prerequisites installed, `make dos` builds the C backend
and `make dos-msfortran` builds `S2F.EXE`, `S2NUM.EXE`, and `S2REG.EXE`.
Keep the three Fortran-backend executables together in a writable directory.

## Design constraints

Vectors contain at most 2048 elements; input files contain at most 30000
bytes. Memory is collected between top-level expressions. Small programs
fit DOS conventional memory; a long loop in one expression can exhaust it.
The replacement RNG is reproducible but does **not** reproduce S2's original
Super-Duper sequence. S3 method dispatch, S4 classes, formula modeling,
full database search paths, and user C/Fortran loading are not implemented.
See the feature set before porting a larger S program.
