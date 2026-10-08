# Runtime and DOS layout

The parser produces an AST; the evaluator operates on immutable/copied
values and local, expression, session, and working environments. Supplied
function arguments retain caller frames as promises; default expressions
retain callee frames. Collection marks ASTs, values, attributes, bindings,
and promise environments between top-level expressions.

Working bindings are checkpointed before each expression. Successful
expression bindings commit; errors restore the checkpoint. External file
writes have normal filesystem effects. ASCII serialization reconstructs
values and functions as S source.

## Executables, segments, and numerical workers

DOS builds are **MZ EXEs**, not COM binaries. Watcom `-ml -0 -bt=dos -lr`
uses multiple code/data segments with far pointers on an 8086 and software
floating point. Individual allocations still fit a segment; conventional
memory remains finite. No DOS extender is required.

| Program | Responsibility | Numerical interface |
| --- | --- | --- |
| `S2.EXE` | Complete implemented interpreter profile | Portable C algorithms |
| `S2F.EXE` | Same interpreter, optional F77 backend | Starts worker EXEs through DOS command interpreter |
| `S2NUM.EXE` | Mean/sample variance worker | `S2NUM.IN`, `S2NUM.OUT` |
| `S2REG.EXE` | Pivoted, twice-reorthogonalized QR worker | `S2REG.IN`, `S2REG.OUT` |
| `s2-f77` (Linux) | Native interpreter with GNU Fortran | Direct calls to `s2stat_`, `s2qr_` |

Separate EXEs avoid incompatible Watcom/Microsoft runtime ABIs. Plain DOS
has no standard Windows-style DLL loader. More workers can be added later;
overlays or an extender would be separate target profiles, not implicit
requirements for an 8086 build.

The F77 solver overwrites a workspace copy of the design with its
orthogonal basis and computes residuals before coefficient back-substitution.
This reduces static data. The DOS driver allows 1024 rows and 2048 design
cells; the native driver permits 2048 rows/cells. Microsoft sources are
compiled separately to avoid the combined-unit compiler failures observed
with this historical compiler. `/FPc /Ot` uses software floating point.

Each DOS F77 process needs an exclusive writable working directory; fixed
worker filenames do not support concurrent sessions in one directory.
Compiler binaries, manual text, and historical proprietary code are not
redistributed in this repository.
