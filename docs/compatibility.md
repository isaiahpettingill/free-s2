# S2 compatibility matrix

Target: the 1988 New S documented by Becker, Chambers and Wilks, **not**
S-PLUS 8 or an R subset. This is a requirements ledger, not a claim of
conformance. No original S2 executable has been tested.

## Evidence register

| ID | Source | Evidence available | Use |
| --- | --- | --- | --- |
| B88 | [The New S Language (1988), publisher](https://www.routledge.com/The-New-S-Language/Becker/p/book/9781315895888) | User-supplied 2018 reissue preview and complete OCR ODT, including illustrated copy with 87 embedded images | Primary target evidence; cite printed pages and sections |
| H20 | [Chambers, S, R, and Data Science (2020), §3.1](https://journal.r-project.org/articles/RJ-2020-028/) | Author's history | First-class functions; separates 1988 from 1992 modeling/classes |
| P10 | [TIBCO Spotfire S+ 8.2 Programmer's Guide (November 2010)](https://www.msi.co.jp/solution/splus/download/pdf/pg.pdf) | Public vendor manual inspected | Later-family evidence only; cannot establish S2 conformance |
| RFAQ | [R FAQ, §§3.1, 3.3](https://stat.ethz.ch/R-manual/R-devel/doc/manual/R-FAQ.html) | R project's comparison manual | Version names and divergence warnings, not a substitute S2 specification |
| RLANG | [R Language Definition, §§4.3.3–4.3.4](https://mac.r-project.org/man/R-lang.html) | R project's language manual | Comparative evidence only |

P10 page references below are **printed pages**, not PDF page numbers
(the PDF has 11 preceding pages). Manual extracts and proprietary source
are not copied into this repository. Tests are independently written.

Evidence grades: **B** = inspected 1988 text in the supplied book; **H** = author confirms historical feature; **L** = later
manual documents behavior but 1988 applicability is unverified; **U** =
1988 clause unavailable. Every L row requires a B88 check before being
promoted to historically verified. Implementation tests establish our
behavior, not the behavior of an unavailable historical executable.

## Language requirements

| ID | Feature / candidate rule | Manual locator | Grade | Implementation / validation |
| --- | --- | --- | --- | --- |
| SYN01 | Expressions, comments, separators; reserved names; `**` synonym | B88 §§11.1.1–11.1.2, pp.338–341 | B | Basic parser implemented; comment retention, `**`, repeat and complete escapes pending |
| SYN02 | Operator precedence, equal levels left-associated except assignment | B88 pp.39–40, Table 3.1 | B | Implemented and tested; generic `%operator%` pending |
| SYN03 | `<-`, `_`, `->`; `=` names arguments | B88 pp.36–41 | B | Implemented and tested |
| VAL01 | Numeric/logical/character vectors, lists | B88 ch.5 | B | Implemented bounded subset |
| VAL02 | Integer/complex/storage modes | B88 ch.5, ch.8 | B | Deferred; not complete mode fidelity |
| VAL03 | NULL/zero-length objects | B88 ch.5; §11.4 | B | Basic values implemented; replacement boundary behavior pending |
| VAL04 | Missing values; division by zero gives NA | B88 pp.41–42; §11.4.4 | B | Numeric sentinel implementation; cannot represent every finite double |
| VEC01 | Recycling/coercion and fractional recycling warning | B88 p.39; pp.363–364 | B | Numeric recycling implemented; full character/logical coercion and attribute rules pending |
| IDX01 | Positive/negative/logical subscripts; fractional truncation | B88 pp.102–104, 357–359 | B | Basic subset implemented |
| IDX02 | `[`, `[[`, `$` distinctions | B88 pp.361–363 | B | Partial; names, atomic `$`, recursive replacement pending |
| IDX03 | NA/out-of-range extraction returns missing atomic / NULL recursive | B88 pp.357–359 | B | Current conservative errors are a known incompatibility |
| IDX04 | Replacement extends/coerces; NA consumes RHS slot; zero-length RHS changes nothing | B88 pp.357–363 | B | Current same-mode bounded replacement is incomplete; no R-style NULL deletion |
| ATT01 | Names, dimensions, general attributes | B88 ch.5; §11.4 | B | Names and 2D dimensions subset; general attributes pending |
| MAT01 | Column-major arrays, first dimension fastest | B88 pp.359–361 | B | 2D numeric matrices implemented; arbitrary arrays pending |
| FUN01 | First-class functions | B88 ch.6; §11.2 | B | Implemented; no lexical capture |
| FUN02 | Supplied promise in caller, default in callee, evaluated at most once | B88 pp.342–345 | B | Implemented; original expression retained; escaping-frame behavior needs probes |
| FUN03 | Exact, unique prefix, then positional argument matching | B88 pp.354–356 | B | Implemented without dots |
| FUN04 | `...`; formals after dots match exactly only | B88 pp.354–356 | B | Deferred |
| ENV01 | Local, expression frame, session frame, cache, search directories; function lookup skips nonfunctions | B88 pp.117–123, 342–345 | B | Local/global subset; skip nonfunction bindings implemented; full search pending |
| ENV02 | Assignments commit after successful top-level expression; errors roll back pending global writes | B88 pp.117–123, 345–347 | B | Top-level binding rollback implemented/tested; persistent directories and session frames pending |
| CTL01 | Loops return last completed body; for restores existing local loop variable | B88 pp.348–351 | B | Loop restoration and last completed values implemented/tested; arbitrary nested control-transfer cases pending |
| META01 | `missing`, `substitute`, expression/parse/deparse/eval | B88 pp.352–356; ch.7 | B | Expression/eval subset; quote is a provisional extension, not established S2 |
| IO01 | Source, text input/output, persistent workspace | B88 ch.5, ch.8 | B | Script runner and REPL implemented; workspace persistence pending |
| STAT01 | mean(x, trim=0); var(x,y); sum(...), prod(...) | B88 pp.505–506, 601–602, 633–634 | B | C/F77 mean and sample variance; trim/covariance/signatures incomplete; na.rm extension not historical |
| STAT02 | Distributions, RNG, quantiles | B88 ch.8, appendix | B | Deferred; original RNG must be reconstructed rather than replaced silently |
| FFI01 | User C/Fortran interfaces | B88 ch.11 | B | Internal F77 backend implemented; historical user-facing FFI deferred |
| GFX01 | Graphics, coordinate systems and devices | B88 ch.4, ch.10; supplied embedded figures | B | Deferred; figures available to check geometry and labels, not device pixels |
| VER01 | S3 modeling/method dispatch, S4 formal classes | H20 §3.1; RFAQ §3.1 | H | Outside target; basic attributes remain in scope |

Implementation tests establish this interpreter's behavior; they do not
establish agreement with an original S executable. The complete manual
has now replaced later-manual inference for the clauses above.

## Historical ambiguities and decisions

| ID | Uncertainty / conflicting evidence | Working decision | What resolves it |
| --- | --- | --- | --- |
| A01 | “Local/global” is an oversimplification. P10 describes additional top-level/session/search frames; RLANG's contrast with lexical closures does not specify every S2 frame. | No lexical closure capture; implement local/global initially and label the missing search path. | B88 ch.11 examples and original frame/search documentation |
| A02 | Later S and R both describe delayed arguments, but that does not prove every 1988 promise rule, especially escaped arguments and defaults. | Supplied expressions use caller frame, defaults callee frame; mark provisional. | B88 argument/evaluation sections and historical tests |
| A03 | Resolved from supplied B88 pp.39–40: equal precedence is left-to-right except assignment; all four AND/OR operators share a level. | Implement left-associated powers and common logical precedence; unary minus binds above colon and below powers. | Regression probes `2^3^2`, `-2^2`, `-1:3`, `T | F & F`; full ch.11 still pending |
| A04 | A single global store omits working-directory databases and assignment commitment. Immediate `<<-` may differ after an error. | Expose current simplified policy; do not claim workspace fidelity. | B88 assignment/search/commitment clauses; side-effect/error probes |
| A05 | P10 p.504 describes restoring a pre-existing loop variable. R generally leaves the final loop value. Is restoration already S2? | Restoration test remains pending; implementation must record its chosen behavior. | B88 loop section or original S2 observation |
| A06 | RFAQ's “S” comparison explicitly addresses S3/S4 engines. List deletion, coercion, empty vectors and `missing()` differences cannot be dated to S2 from it alone. | Use R only for arithmetic smoke comparisons, never as universal oracle. | Version-specific B88 clauses and dated release manuals |
| A07 | 1984 Old S, 1988 New S/S2, and “old S engine” in RFAQ refer to different boundaries. S-PLUS product versions do not equal S language versions. | Tag every source with publication and language generation. | Dated manuals/release notes, not product names alone |
| A08 | Basic class/attribute terminology is older than the 1992 modeling method system. Excluding all “classes” could remove legitimate S2 data behavior. | Exclude S3 dispatch and S4 formal classes, not ordinary metadata. | B88 data/object chapters and function index |
| A09 | Publisher lists 720 pages; bibliographic records list 702. Reprints/digital pagination may differ. | Cite edition, chapter and printed page; no invented B88 page numbers. | Obtain identified edition and record its pagination |
| A10 | Unix numerical results may depend on floating point, libraries, RNG and rounding. DOS cannot promise identical last bits. | Use documented tolerances; state 16-bit bounds separately. | B88 numerical contracts and dated algorithm references |
| A11 | Partially resolved from B88 pp.36–38: underscore is assignment, names use letters/digits/periods, `=` names arguments. Escapes and complete name grammar remain pending. | Implement underscore assignment, reject embedded underscores/top-level `=`; allow system names beginning with dot (B88 p.50). | B88 lexical grammar |

## Validation policy

Each implemented row must acquire an independent test and its Linux/DOS
result. Track evidence separately from test success. Pending historical
probes must not be represented as passing conformance tests. Update this
ledger when a 1988 clause is available, retain the previous provisional
choice in the ambiguity history, and change tests if the evidence warrants
it. Never read proprietary interpreter source to fill a specification gap.

## Preview provenance and newly resolved details

The supplied file is `preview-9781351083430_A37609949.pdf`, 73 PDF
pages, identifying the 1988 text reissued in 2018. Printed p.40 is PDF
page 59; its operator table was checked visually because OCR drops
operator glyphs. The preview ends at printed p.54, despite a contents
list for later chapters: **chapters 6, 7 and 11 are not present**.

B88 p.35 says control AND/OR may return the second operand directly,
not merely a coerced logical scalar. B88 pp.45–46 identify sample
variance and illustrate denominator n−1. B88 p.53 explicitly treats
updated online help as authoritative over the printed appendix for
installed objects: “1988 book compatibility” and any later installed
S runtime therefore need separate version labels.

## Full-book evidence update (2026-10-08)

The user subsequently supplied `the-new-s-language-text-only.odt` and
`the-new-s-language.odt`. Both identify the 1988 book reissued in 2018.
The illustrated copy contains 87 embedded raster images. Its source-page
markers permit checking printed-page offsets (printed p.40 = source PDF
p.59). OCR merges words and can lose operator glyphs, so code fragments,
equations and chart labels need visual verification where images exist.
The files themselves are reference material and are not redistributed.

The earlier preview-only uncertainties remain above as an audit trail;
the following findings supersede their provisional decisions:

| Previous ID | Resolution from full 1988 book | Remaining uncertainty / action |
| --- | --- | --- |
| A01 | pp.117–123 explicitly distinguish local, expression, session, cache and database lookup | Implement frame/search lifecycle; do not call two stores complete S scoping |
| A02 | pp.342–345 establish caller/default frames and memoization | Check substitute after forcing and escaped promises |
| A04 | pp.117–123 require deferred commitment and rollback on error | Add transactional top-level writes; persistent databases later |
| A05 | pp.348–351 require loop binding restoration and last completed body value | Correct evaluator and independently test control transfers |
| A06 | pp.357–363 establish missing extraction and zero-length replacement no-op | Correct indices/coercion; do not import R list-deletion behavior |
| A11 | pp.338–341 list reserved T/F and lexical grammar | Reserved assignments rejected/tested; complete escapes and operator spelling pending |
| A12 (new) | p.35 describes the second operand as the result of control AND/OR; p.354 formal model applies Logical.value to it | Internal printed-book conflict. Implemented/tested ch.11 logical-result model; retain opposing clause and probe an original runtime if found |
| A13 (new) | mean/cor descriptions specify single-precision calculations while backend uses double | Deliberate numerical portability difference; document tolerances rather than claim bit identity |
| A14 (new) | p.53 makes installed online help authoritative for installed objects | Book target is fixed 1988 publication; vendor/runtime updates are separate dialect evidence |

Chart reconstruction should record figure/page, input data, plotting calls,
axes, coordinates and clipping. Raster resemblance alone cannot establish
historical graphics-device compatibility. Graphics remain a later phase.
