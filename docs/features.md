# Feature set

free-s2 implements a compact statistical language and portable file-based
charts. “Supported” means independently tested here; it does not mean
observed agreement with an original 1988 executable.

| Area | Available |
| --- | --- |
| Programs | Interactive prompt, multiline input, script files, `-e`, `source`, error recovery |
| Syntax | `<-`, `_`, `->`, `<<-`; arithmetic, comparisons, logical operators, `**`, custom `%operator%`; comments |
| Control flow | `if`/`else`, `for`, `while`, `repeat`, `break`, `next`, `return` |
| Values | Numeric, logical, character, complex, integer-marked vectors, lists, NULL, NA, matrices, arrays |
| Data access | One-based positive, negative, logical, and named indices; `[`, `[[`, `$`; replacement and extension |
| Metadata | `names`, `dim`, `dimnames`, `attr`, `attributes`, `class`, `structure` |
| Functions | First-class functions, delayed memoized arguments, defaults, exact/prefix/positional matching, `...`, `missing`, `nargs` |
| Computation on groups | `lapply`, `sapply`, matrix `apply`, `category`, `levels`, `cut`, `table`, `split`, `tapply` |
| Language objects | `expression`, `parse`, `deparse`, `substitute`, `eval`; `quote` is an explicit convenience extension |
| Files | `scan`, `write`, `cat`, `paste`, `dump`, `restore`, `dput`, `dget` |
| Statistics | `mean` with trimming, sample `var`, `cor`, `median`, S2 `quantile`, numeric `summary`, `lsfit` |
| Matrices | `matrix`, `array`, `t`, `cbind`, `rbind`, `diag`, `%*%`, `%o%`, `crossprod`, `outer`, `solve` |
| Charts | SVG and PostScript; `plot`, `points`, `lines`, `abline`, `text`, `title`, `hist`, `barplot`, `boxplot`, `qqnorm`, `qqplot` |

## Everyday functions

Vectors: `c`, `list`, `seq`, `rep`, `rev`, `length`, `sort`, `order`, `rank`,
`unique`, `duplicated`, `match`, `pmatch`, `%in%`, `diff`, `cumsum`, `cumprod`,
`cummin`, `cummax`. `order` currently sorts one key; `apply` handles matrices.

Coercion: `as.numeric`, `as.integer`, `as.logical`, `as.character`,
`as.complex`, `as.list`, `as.vector`, `as.matrix`, `as.array`, `as.expression`.
Constructors: `numeric`, `integer`, `logical`, `character`, `complex`, `vector`.
Predicates include `is.na`, `is.numeric`, `is.logical`, `is.character`,
`is.complex`, `is.list`, `is.null`, `is.function`, `is.matrix`, `is.array`,
`is.expression`, `is.call`, `is.name`, `is.category`.

Math: `sum`, `prod`, `min`, `max`, `range`, `any`, `all`, `abs`, `sqrt`,
`exp`, `log`, `log10`, `sin`, `cos`, `tan`, inverse and hyperbolic real
functions, `floor`, `ceiling`, `trunc`, `round`, `signif`, `sign`, `atan2`,
`pmin`, `pmax`, `gamma`, `lgamma`, `beta`, `lbeta`, `Re`, `Im`, `Mod`, `Arg`,
`Conj`. Complex transcendental support covers sqrt/log/exp/sin/cos/sinh/cosh;
unsupported complex operations produce errors.

Strings: `paste`, `nchar`, `substring`/`substr`. Workspace: `objects`/`ls`,
`get`, `exists`, `assign`, `remove`/`rm`. `help(mean)` prints the callable
signature and directs you to these documents.

## Distributions

Every family below has `d` density/mass, `p` lower-tail probability, `q`
quantile, and `r` random sampling functions. Use their 1988-style parameters.
For instance, `pnorm(0)`, `qnorm(.975)`, `dbinom(0:4,4,.5)`, `rnorm(20)`.

| Family suffix | Parameters after x/q/p/n |
| --- | --- |
| `norm` | mean, sd |
| `unif` | min, max |
| `exp` | none; standard exponential |
| `binom` | size, prob |
| `pois` | lambda |
| `t`, `chisq` | df |
| `gamma` | shape; unit scale |
| `beta` | shape1, shape2 |
| `f` | df1, df2 |
| `cauchy`, `logis` | location, scale |
| `weibull` | shape; unit scale |

`sample(x,size,replace)` samples bounded populations. `set.seed(i)` accepts
0–1000; saving/restoring `.Random.seed` replays this implementation's stream.
It is a replacement Park–Miller generator and modern sampling algorithms.

## Limits and incomplete compatibility

| Constraint | Current behavior |
| --- | --- |
| Vector/array size | 2048 elements, including a matrix's total cells |
| Source text | 30000 bytes per input/parse/source; strings at most 4096 bytes |
| Calls | 64 arguments/formals; 128 evaluation depth |
| Arrays | Up to 16 dimensions; apply currently two dimensions |
| Least squares | At most 32 coefficients; DOS Fortran at most 1024 rows; full-rank design required; coefficients/residuals available, `fit$qr` is NULL |
| Integer storage | Integer flag and truncating coercion; numeric storage remains double, not original machine integer representation |
| Missing/infinite values | NA uses a reserved double sentinel; infinite quantile endpoints are reported as NA |
| Workspace | Explicit ASCII dump/restore; no automatic `.Data` database, attach/detach, database search path, or historical binary files |
| Extended evaluation | eval with a list or FALSE works; arbitrary frame numbers and parent frames are unsupported |
| Sorting | Missing-value sort policy is unresolved and produces an error |
| File scanning | Simple whitespace or separated fields with basic quoting; not a general CSV dialect parser |
| Graphics | Core geometry and labels; limited graphical parameters, no interactive DOS screen device; SVG stores one plot per file, PostScript supports pages |
| Numerics | Double precision and independent algorithms; no historical bit-for-bit or RNG claim |
| Memory | Collection between expressions; conventional-memory exhaustion possible in large single expressions |

S3/S4 dispatch, formula-based `lm`/`glm`, the full 1988 statistical library,
advanced time-series/multivariate tools, debugger, and user FFI remain outside
this implemented profile. They must not be inferred from a later S-PLUS or
R tutorial. The compatibility matrix records the evidence for future work.
