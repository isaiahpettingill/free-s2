# Using free-s2

A compact guide to the implemented language. Start with runnable examples,
then use the [feature set](features.md) and [historical matrix](compatibility.md)
when moving an existing S program.

## Contents

1. [Build and run](#build-and-run)
2. [Values and vectors](#values-and-vectors)
3. [Selecting and changing data](#selecting-and-changing-data)
4. [Functions and control flow](#functions-and-control-flow)
5. [Matrices and grouped data](#matrices-and-grouped-data)
6. [Statistics and simulation](#statistics-and-simulation)
7. [Read, save, and restore](#read-save-and-restore)
8. [Charts](#charts)
9. [Language objects](#language-objects)
10. [DOS](#dos)

## Build and run

Linux needs a C compiler, make, and Python 3 for the tests:

```sh
make test
./build/s2 --version
./build/s2 -e 'mean(1:10)'
./build/s2 examples/HELLO.S
./build/s2
```

GNU Fortran is optional: `make test-fortran` builds `build/s2-f77` with the
F77 numerical backend. Use `q()` to leave the prompt. Unfinished parentheses,
braces, strings, or trailing operators produce a continuation prompt.
Successful top-level assignments persist in the running session; an error
rolls back assignments made by that top-level expression. Files and charts
already written are ordinary external side effects and are not rolled back.

## Values and vectors

```s
x <- c(2,4,6,8)
x + 1
x * c(1,2)
length(x)
1:5
seq(0,1,by=.25)
rep(c("yes","no"),length=5)
z <- c(1+2i,3i)
Re(z)
```

Arithmetic works element by element. Short vectors recycle; fractional
recycling issues a warning. Indices begin at 1. Use `<-` to assign: top-level
`=` is not assignment in this dialect. Names may contain letters, digits,
and periods; underscore is the historical assignment operator.

`T`/`F` and `TRUE`/`FALSE` denote logical values. `NA` is missing data, and
`NULL` is an empty object. Select complete values before calculating:

```s
x <- c(1,NA,3)
is.na(x)
mean(x[!is.na(x)])
```

The modern R `na.rm` convention is not a documented argument to S2's `sum`.
`sum(c(1,NA),na.rm=T)` therefore does not remove missing data here.

## Selecting and changing data

```s
x <- c(low=10,mid=20,high=30)
x[c(1,3)]
x[-2]
x[x > 10]
x["mid"]
x[4] <- 40
names(x) <- c("low","mid","high","extra")
d <- list(score=x,label="trial")
d$score
 d[["label"]]
d$score[2] <- 25
```

`[` returns a subset; `[[` returns an individual list component; `$` selects
a named component. Atomic missing/out-of-range extraction returns NA;
list extraction returns NULL where the manual specifies it. Replacement
can extend a vector and coerce its mode. Character subscripts can partially
match on extraction; replacement uses exact names.

## Functions and control flow

```s
center <- function(x) x - mean(x)
center(1:5)
shift <- function(x,amount=1) x + amount
shift(1:3,amount=2)
total <- function(...,offset=0) sum(...) + offset
total(1,2,3,offset=10)
s <- 0
for(i in 1:5) s <- s + i
s
```

Functions are values. Arguments are delayed until needed, then memoized;
supplied expressions use the caller's frame and defaults use the callee's.
Named arguments match exactly, then by unique prefix, then positionally.
Arguments after `...` require exact names. `missing(x)` asks whether a formal
was supplied, even if its default was evaluated.

Use `if`/`else`, `while`, `repeat`, `break`, `next`, and `return`. A for-loop
restores a pre-existing local loop binding. A function's last expression is
its result unless it returns explicitly.

Original S lookup does not capture a lexical enclosing function environment.
An inner function reads its own arguments/locals and the expression/session/
working workspace. Do not expect R closures. `<<-` writes the working
workspace, subject to top-level assignment rollback.

## Matrices and grouped data

```s
m <- matrix(1:6,nrow=2)
m[2,3]
m[,2,drop=F]
t(m)
colmeans <- apply(m,2,mean)
colmeans
solve(matrix(c(2,0,0,4),2),c(6,8))
g <- category(c("north","south","north","south"))
levels(g)
table(g)
tapply(c(3,5,7,9),list(g),mean)
```

Matrices and arrays store data column first. `dim` and `dimnames` can be
assigned. `array(1:8,c(2,2,2))` makes a three-dimensional object.
`%*%` multiplies matrices; `%o%` and `outer` form pairwise results.
Categories store numeric codes with levels; `as.character(g)` recovers labels.
Use `lapply` to retain a list or `sapply` to simplify compatible results.

## Statistics and simulation

```s
x <- 1:5
y <- c(3,5,7,9,11)
mean(y)
var(y)
cor(x,y)
quantile(y)
fit <- lsfit(x,y)
as.vector(fit$coef)
fit$residuals
pnorm(0)
qnorm(.975)
set.seed(42)
rnorm(5)
```

`var` uses the sample denominator n−1. `quantile` uses the S2 interpolation
positions (i−0.5)/n; results can differ from R's default quantiles.
`lsfit` supports full-rank small designs, optional weights, intercept, and
multiple response columns. Its coefficient/residual fields are usable;
the historical QR return object has not been reconstructed.

Random streams repeat with the same seed in free-s2. They do not reproduce
original S2 streams. Distribution functions are independent numerical
implementations; use the documented limits before relying on extreme tails.

## Read, save, and restore

For `DATA.TXT` containing rows such as `1 3 north`:

```s
d <- scan("DATA.TXT",what=list(x=0,y=0,region=""))
mean(d$y)
write(d$y,"Y.TXT",ncolumns=1)
cat("Average:",mean(d$y),"\n",file="REPORT.TXT")
dump(objects(),"WORK.S")
```

A new session can call `restore("WORK.S")`. Functions, lists, names, and
attributes are saved as readable S source; there is no implicit startup
restore or exit save. Use `dput(x,"X.S")` and `x <- dget("X.S")` for one
object. `source("PROGRAM.S")` runs a script inside the current session.
Dump files must fit the source limit to restore in one call.

## Charts

```s
svg("FIT.SVG")
plot(x,y,main="Response",xlab="Input",ylab="Output")
abline(fit,col="red")
dev.off()
postscript("FIT.PS")
plot(x,y,type="b")
dev.off()
```

Always close the device to complete the file. Normal program exit closes it
as well. SVG is portable to modern viewers; PostScript is useful for old
printing/viewing tools. `plot` types include points (`"p"`), lines (`"l"`),
both (`"b"`), and an empty panel (`"n"`). Add `points`, `lines`, `text`,
`abline`, or `title`. Histogram bins use right-closed intervals:

```s
svg("HIST.SVG")
hist(y,nclass=5,main="Responses")
dev.off()
```

`breaks` expects an explicit vector of boundaries. `barplot`, `boxplot`,
`qqnorm`, and equal-length `qqplot` are available. This implements core
geometry, not the full historical device or graphical-parameter system.

## Language objects

```s
e <- expression(x <- 2,x + 3)
eval(e)
e <- parse(text="1 + 2")
eval(e)
f <- function(x) deparse(substitute(x))
f(1 + 2)
```

`substitute` retains the original argument expression even after forcing.
`eval(e,list(x=5))` supplies local values; `eval(e,FALSE)` uses the working
workspace. Extended numeric/parent frames are not implemented.

## DOS

With Open Watcom installed, build `make dos-local`. Copy `build/S2.EXE`
and an example script/data to DOS, then run:

```text
S2 HELLO.S
S2 ANALYZE.S
S2
```

Use short 8.3 filenames for broad compatibility. `S2F.EXE` is the optional
Microsoft FORTRAN backend (regression limited to 1024 rows); put `S2NUM.EXE` and `S2REG.EXE` beside it. Worker
files use fixed names, so each running process needs an exclusive writable
working directory. Charts are exported files, not live VGA windows.

DOSBox checks use the requested dosbox-agent-tools PowerShell module.
See [validation](validation.md) for the build flags and test procedure.
