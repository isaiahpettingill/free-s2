# free-s2

Independent implementation targeting the 1988 New S language (S2), in
Open Watcom-compatible C89 with optional Fortran 77 numeric routines.

Start with the [manual-derived compatibility ledger](docs/compatibility.md),
including historical ambiguities and the provenance of each requirement.
The full 1988 manual is available as user-supplied reference material,
including its charts. The ledger separates documented requirements from
implemented behavior and records internal contradictions in the book.

Status: initial development. Linux and 8086 real-mode DOS are the targets.


Build and run the initial subset on Linux:

```sh
make test
./build/s2 -e 'mean(1:10)'
make test-fortran
```

With Open Watcom installed, `make dos-local` builds an 8086 real-mode
large-model DOS executable. `make dos` uses the sibling
[dosbox-agent-tools](https://github.com/isaiahpettingill/dosbox-agent-tools)
compiler wrapper. `make dos-msfortran` stages the Microsoft FORTRAN worker;
it communicates through files to avoid mixing incompatible C/FORTRAN
runtime ABIs. GNU Fortran uses a direct numeric subroutine call.

This is an initial interpreter, not full S2. See the ledger for known
semantic gaps. Limits: 2048 elements per vector, 30000-byte source input,
128 evaluation depth; collection occurs between top-level expressions.
No persistent workspace or graphics device is implemented yet.

[Validation results and reproduction](docs/validation.md) cover both
native Linux builds and DOSBox runs with C and Microsoft FORTRAN numerics.
