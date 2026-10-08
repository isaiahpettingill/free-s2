# Initial implementation validation

This validates our implementation, not conformance against an original
1988 executable. Fixtures are independently written and carry source
locators in `tests/cases.json`. The compatibility ledger lists omissions.

| Target | Compiler/backend | Result |
| --- | --- | --- |
| Native Linux | GCC 13, strict C89, C numeric routines | 48 fixtures + interactive recovery and rollback passed |
| Native Linux | GCC 13 + GNU Fortran 13, fixed-form F77 | Same 48 fixtures + interactive checks passed |
| Native Linux numeric worker | GNU Fortran 13 | Five independent Python statistics comparisons passed |
| DOSBox | Open Watcom, 8086 `-0`, DOS real mode, large model, C numerics | 48 fixtures passed |
| DOSBox | Open Watcom + Microsoft FORTRAN 5.0 `/FPc /Ot` standalone worker | 48 fixtures passed |

DOS tests used the PowerShell module from `dosbox-agent-tools`, with
DOSBox automation 0.85.1 (7567ec4). Microsoft compiler files came from
that project's pinned `dos_compilers` submodule at
`da3f15ce4912893a0dd9f8349dab29116a53ca59`. Compilers and book files are
not redistributed here. No physical 8086 system has been tested.

Open Watcom was invoked directly because container tooling was unavailable
on the test host. Microsoft FORTRAN was compiled and executed within
DOSBox. `/FPc` selects software floating point; the process/file bridge
avoids incompatible Watcom/Microsoft runtime linking. GNU Fortran uses
direct `s2stat_` linking. The DOS bridge uses fixed filenames and requires
an exclusive writable working directory with `S2NUM.EXE` present.

## Reproduction

Native: `make test`, `make test-fortran` with the appropriate compilers
available. DOS: build `S2.EXE`, `S2F.EXE` and `S2NUM.EXE`, then:

1. Run `python3 tests/prepare_dos.py`.
2. Configure DOSBox automation to mount `build/` as C: in its primary
   config and permit that host path in its mount allowlist.
3. Launch `tests/dos.ps1` with PowerShell, the DOSBox `-BinaryPath`, and
   its `-TokenFile`. Set `POWERSHELL_TELEMETRY_OPTOUT=1` before launching
   PowerShell. The script uses the sibling dosbox-agent-tools module.
4. Run `python3 tests/check_dos.py` on the host. Output and DOS errorlevel
   are checked; DOSBox creates empty redirection files even when an IF
   branch is false, so file existence alone is not an error indicator.

Bounds and numerical differences are explicit in README and the ledger.
The interpreter collects memory between top-level expressions; long
single-expression loops can exhaust conventional memory. Error text is
checked natively; DOS checks errorlevel and successful output. Interactive
DOS recovery, physical hardware and historical RNG remain untested.
