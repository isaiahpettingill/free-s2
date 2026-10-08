# S2 compatibility matrix

Target: the 1988 New S documented by Becker, Chambers and Wilks, **not**
S-PLUS 8 or an R subset. This is a requirements ledger, not a claim of
conformance. No original S2 executable has been tested.

## Evidence register

| ID | Source | Evidence available | Use |
| --- | --- | --- | --- |
| B88 | [The New S Language (1988), publisher](https://www.routledge.com/The-New-S-Language/Becker/p/book/9781315895888) | Description and chapter list; full manual not yet obtained | Defines target; detailed clauses remain pending |
| H20 | [Chambers, S, R, and Data Science (2020), §3.1](https://journal.r-project.org/articles/RJ-2020-028/) | Author's history | First-class functions; separates 1988 from 1992 modeling/classes |
| P10 | [TIBCO Spotfire S+ 8.2 Programmer's Guide (November 2010)](https://www.msi.co.jp/solution/splus/download/pdf/pg.pdf) | Public vendor manual inspected | Later-family evidence only; cannot establish S2 conformance |
| RFAQ | [R FAQ, §§3.1, 3.3](https://stat.ethz.ch/R-manual/R-devel/doc/manual/R-FAQ.html) | R project's comparison manual | Version names and divergence warnings, not a substitute S2 specification |
| RLANG | [R Language Definition, §§4.3.3–4.3.4](https://mac.r-project.org/man/R-lang.html) | R project's language manual | Comparative evidence only |

P10 page references below are **printed pages**, not PDF page numbers
(the PDF has 11 preceding pages). Manual extracts and proprietary source
are not copied into this repository. Tests are independently written.

Evidence grades: **H** = author confirms historical feature; **L** = later
manual documents behavior but 1988 applicability is unverified; **U** =
1988 clause unavailable. Every L row requires a B88 check before being
promoted to historically verified. Implementation tests establish our
behavior, not the behavior of an unavailable historical executable.

## Language requirements

| ID | Feature / candidate rule | Manual locator | Grade | Implementation / validation |
| --- | --- | --- | --- | --- |
| SYN01 | Expressions, comments, separators | B88 ch.3; P10 pp.495–506 | L | Parser draft; untested |
| SYN02 | Operator precedence | P10 pp.153–155 | L | Parser draft; ambiguity A03 |
| SYN03 | Assignment syntax | P10 pp.8, 502 | L | Parser draft; `=` deliberately unresolved |
| VAL01 | Numeric/logical/character vectors | P10 pp.66–70 | L | Planned |
| VAL02 | Integer/complex/storage modes | B88 ch.5, ch.8 | U | Deferred; must not silently coerce unsupported modes |
| VAL03 | NULL/zero-length objects | P10 ch.4 | L | Planned; boundary cases pending |
| VAL04 | Missing values | B88 ch.5; P10 ch.4 | U | Planned representation; exact propagation pending |
| VEC01 | Recycling/coercion | P10 pp.155–157 | L | Planned; warning details pending |
| IDX01 | Positive/negative/logical subscripts | P10 pp.172–184 | L | Planned |
| IDX02 | `[`, `[[`, `$` distinctions | P10 pp.9–10, 172–184 | L | Planned |
| IDX03 | Out-of-range and missing subscripts | B88 ch.3, ch.8 | U | Conservative errors pending specification |
| IDX04 | Subassignment/list deletion | RFAQ §3.3.3; B88 ch.8 pending | U | Deferred |
| ATT01 | Names, dimensions, attributes | P10 pp.71–79 | L | Planned subset; preservation pending |
| MAT01 | Column-major matrices | P10 pp.71–76, 158 | L | Planned |
| FUN01 | First-class function definitions | B88 description, ch.6; H20 §3.1 | H | Parser draft; evaluator planned |
| FUN02 | Delayed arguments/default frames | P10 pp.28, 199–204; RLANG §4.3.3 | L | Planned; A02 |
| FUN03 | Exact/partial/positional matching | P10 pp.61–62, 199–204 | L | Planned exact/positional; partial matching deferred |
| FUN04 | `...` and forwarding | P10 pp.199–204 | L | Deferred |
| ENV01 | Local/top-level/session/search lookup | P10 pp.23–29; RLANG §4.3.4 | L | Planned local/global subset; A01 |
| ENV02 | Global assignment/commitment | P10 pp.40–42, 502 | L | Deferred full commitment; A04 |
| CTL01 | Branches, loops, returns | P10 pp.185–198, 503–504 | L | Planned subset; A05 |
| META01 | Expression objects, parse/deparse/eval | B88 description, ch.7, ch.11 | H/U | Planned quote/eval subset; no full AST manipulation |
| IO01 | Source, text input/output, workspace | B88 ch.5, ch.8, ch.11 | U | Planned script runner; persistence deferred |
| STAT01 | Summary/numerical routines | B88 ch.9/function reference pending | U | Planned mean/variance; signatures pending |
| STAT02 | Distributions, RNG, quantiles | B88 function reference pending | U | Deferred until algorithms/signatures are specified |
| FFI01 | C/Fortran interfaces | B88 description, ch.11 | H/U | Portable numeric backend planned; user FFI deferred |
| GFX01 | Graphics/devices | B88 ch.4, ch.10 | H/U | Deferred |
| VER01 | S3 dispatch/modeling, S4 formal classes | H20 §3.1; RFAQ §3.1 | H | Outside 1988 target; basic attributes not excluded |

The B88 chapter pointers are navigation aids from its table of contents,
**not claims that the chapter's detailed specification was read**. H/U
means feature presence is confirmed but exact semantics are unresolved.

## Historical ambiguities and decisions

| ID | Uncertainty / conflicting evidence | Working decision | What resolves it |
| --- | --- | --- | --- |
| A01 | “Local/global” is an oversimplification. P10 describes additional top-level/session/search frames; RLANG's contrast with lexical closures does not specify every S2 frame. | No lexical closure capture; implement local/global initially and label the missing search path. | B88 ch.11 examples and original frame/search documentation |
| A02 | Later S and R both describe delayed arguments, but that does not prove every 1988 promise rule, especially escaped arguments and defaults. | Supplied expressions use caller frame, defaults callee frame; mark provisional. | B88 argument/evaluation sections and historical tests |
| A03 | P10's prose says equal-precedence operators proceed left to right. Modern R's exponentiation association must not be imported automatically. | Keep association probes pending; do not certify parser association from R agreement. | B88 grammar/precedence table, especially `2^3^2`, `-2^2`, `-1:3` |
| A04 | A single global store omits working-directory databases and assignment commitment. Immediate `<<-` may differ after an error. | Expose current simplified policy; do not claim workspace fidelity. | B88 assignment/search/commitment clauses; side-effect/error probes |
| A05 | P10 p.504 describes restoring a pre-existing loop variable. R generally leaves the final loop value. Is restoration already S2? | Restoration test remains pending; implementation must record its chosen behavior. | B88 loop section or original S2 observation |
| A06 | RFAQ's “S” comparison explicitly addresses S3/S4 engines. List deletion, coercion, empty vectors and `missing()` differences cannot be dated to S2 from it alone. | Use R only for arithmetic smoke comparisons, never as universal oracle. | Version-specific B88 clauses and dated release manuals |
| A07 | 1984 Old S, 1988 New S/S2, and “old S engine” in RFAQ refer to different boundaries. S-PLUS product versions do not equal S language versions. | Tag every source with publication and language generation. | Dated manuals/release notes, not product names alone |
| A08 | Basic class/attribute terminology is older than the 1992 modeling method system. Excluding all “classes” could remove legitimate S2 data behavior. | Exclude S3 dispatch and S4 formal classes, not ordinary metadata. | B88 data/object chapters and function index |
| A09 | Publisher lists 720 pages; bibliographic records list 702. Reprints/digital pagination may differ. | Cite edition, chapter and printed page; no invented B88 page numbers. | Obtain identified edition and record its pagination |
| A10 | Unix numerical results may depend on floating point, libraries, RNG and rounding. DOS cannot promise identical last bits. | Use documented tolerances; state 16-bit bounds separately. | B88 numerical contracts and dated algorithm references |
| A11 | `=` assignment, underscores in identifiers, escapes and modern literal spellings may be extensions. | Syntax acceptance is provisional, not evidence of historical legality. | B88 lexical grammar |

## Validation policy

Each implemented row must acquire an independent test and its Linux/DOS
result. Track evidence separately from test success. Pending historical
probes must not be represented as passing conformance tests. Update this
ledger when a 1988 clause is available, retain the previous provisional
choice in the ambiguity history, and change tests if the evidence warrants
it. Never read proprietary interpreter source to fill a specification gap.
