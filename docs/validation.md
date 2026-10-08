# Validation

Checked on 2026-10-08. These tests validate this independent implementation,
not conformance against an original 1988 executable. The compatibility
matrix separates evidence, known differences, and unresolved behavior.

| Target | Compiler/backend | Result |
| --- | --- | --- |
| Native Linux | GCC 13, strict C89, portable C numerics | 76 language cases, REPL recovery/rollback, guide snippets, data/graphics/persistence workflows, 23 analytic distribution checks passed |
| Native Linux | GCC 13 + GNU Fortran 13, fixed-form F77 | Same language/workflow/distribution checks passed |
| Native numerical workers | GNU Fortran 13 | Five independent mean/variance cases and three QR exact-fit/rank-deficiency cases passed |
| Native instrumentation | GCC undefined-behavior sanitizer | 76 language cases and data/graphics/serialization workflows passed |
| DOSBox | Open Watcom, 8086 `-0`, real mode, `-ml`, C numerics | 76 language cases and data/regression/simulation/chart workflows passed |
| DOSBox | Open Watcom + Microsoft FORTRAN 5.00 `/FPc /Ot` workers | Same 76 cases and workflows passed, including Fortran regression |

The DOS executables have MZ headers: `S2.EXE` (188892 bytes), `S2F.EXE`
(198208), `S2NUM.EXE` (35698), `S2REG.EXE` (43648) for this tested build.
These are large-model EXEs, not COM programs. Sizes may change by toolchain.

The workflow reads typed columns, computes grouped means and sample variance,
fits exact data with intercept 1/slope 2, writes residuals, and exports SVG
and PostScript. Host tests parse SVG XML and inspect completed PostScript
pages/trailers. Simulation adds histogram and normal QQ charts. Native
persistence tests restore data, functions, names, and class attributes.
All S code blocks in the usage guide execute in a staged example directory.

Analytic distribution checks use independently known identities (normal
CDF via Python erf, gamma/exponential identities, binomial/Poisson masses,
Cauchy/t/F symmetries, and inverse quantiles). These are numerical checks,
not a reference S or R oracle. They do not certify all extreme tails.

## Toolchain provenance

DOS tests use the PowerShell module from the requested
[dosbox-agent-tools](https://github.com/isaiahpettingill/dosbox-agent-tools),
commit `48d6bb2b7962c4334fda5a3a113f0f1724f7bae6`, and DOSBox automation
0.85.1 (`7567ec4`). Microsoft compiler files come from its pinned
`dos_compilers` submodule `da3f15ce4912893a0dd9f8349dab29116a53ca59`.
Compilers and book files are not redistributed. Physical 8086 hardware
has not been tested.

Open Watcom was invoked directly because container tooling was unavailable
on the test host. Microsoft FORTRAN was compiled and run inside DOSBox,
with source files staged beside its BIN/LIB/INCLUDE directories (including
INIT and TMP settings). Loading its passes from a different mounted root
caused internal compiler errors. The regression driver and QR routine are
compiled as separate files, then linked. A tested DOS-specific driver caps
regression at 1024 rows/2048 design cells to keep its runtime data bounded.
Native F77 retains the 2048-row/cell limit. The worker bridge uses uppercase
scientific exponents in its portable text protocol.

`/FPc` selects software floating point; no 8087 is required. Separate worker
processes avoid mixing Microsoft and Watcom runtime ABIs. Native GNU
Fortran links directly to `s2stat_` and `s2qr_`. DOS worker names/files require
an exclusive writable working directory.

## Reproduction

```sh
make test
make test-fortran
```

DOS builds: `make dos-local` with Open Watcom installed, or `make dos` and
`make dos-msfortran` using the sibling compiler containers. The latter
needs the sibling project's pinned compiler submodule and container engine.

1. Stage the executables together in `build/`, then run `python3 tests/prepare_dos.py`.
2. Configure DOSBox automation to mount `build/` as C: and permit that host directory in its primary mount allowlist.
3. Launch `tests/dos.ps1` with PowerShell, DOSBox `-BinaryPath`, and `-TokenFile`. Set `POWERSHELL_TELEMETRY_OPTOUT=1` and `DOTNET_CLI_TELEMETRY_OPTOUT=1` in the parent environment.
4. Run `python3 tests/check_dos.py` on the host. It checks output/errorlevel, data results, SVG parsing and PostScript completion for both backends.

DOSBox can create empty redirection files even when an IF branch is false;
checks use their contents, not mere existence. Error wording is tested
natively; DOS errors are checked by errorlevel.

## Practical limits

No historical executable, original RNG sequence, hardware graphics driver,
full database search path, or physical machine has been compared. DOS
interactive recovery is not separately automated. Garbage collection occurs
between top-level expressions; a large single-expression loop can exhaust
conventional memory. The [feature set](features.md) lists the bounded
implemented profile and the [matrix](compatibility.md) records historical
uncertainties. CI checks native builds; DOS tests currently run through the
explicit emulator workflow above.
