CC = cc
FC = gfortran
CFLAGS = -O2 -std=c89 -pedantic -Wall -Wextra
FFLAGS = -O2 -std=legacy -ffixed-form -Werror -Wall -Wextra
SOURCES = src/main.c src/memory.c src/parser.c src/value.c src/eval.c src/builtin.c src/numeric.c
TOOLS ?= ../dosbox-agent-tools
WCL ?= wcl

.PHONY: all test fortran test-fortran dos dos-local dos-msfortran clean
all: build/s2
build:
	mkdir -p build
build/s2: $(SOURCES) src/s2.h | build
	$(CC) $(CFLAGS) -o $@ $(SOURCES) -lm
build/stats.o: numeric/STATS.FOR | build
	$(FC) $(FFLAGS) -c -o $@ $<
fortran: build/s2-f77
build/s2-f77: $(SOURCES) src/s2.h build/stats.o
	$(CC) $(CFLAGS) -DS2_FORTRAN -o $@ $(SOURCES) build/stats.o -lgfortran -lm
build/s2num: numeric/S2NUM.FOR numeric/STATS.FOR | build
	$(FC) $(FFLAGS) -o $@ $^
test: build/s2
	python3 tests/check.py build/s2
test-fortran: build/s2-f77 build/s2num
	python3 tests/check.py build/s2-f77
	python3 tests/numeric.py build/s2num
dos: | build
	$(MAKE) -C $(TOOLS)/toolchains/openwatcom compile WORKDIR=$(CURDIR) ARGS='-ml -k16384 -fe=build/S2.EXE $(SOURCES)'
# Direct profile for hosts with Open Watcom already installed.
dos-local: | build
	cd build && $(WCL) -q -bt=dos -lr -ml -0 -k16384 -fe=S2.EXE $(addprefix ../,$(SOURCES))
dos-msfortran: | build
	cat numeric/S2NUM.FOR numeric/STATS.FOR > build/S2NUM.FOR
	$(MAKE) -C $(TOOLS)/toolchains/msfortran compile WORKDIR=$(CURDIR)/build PROGRAM=S2NUM.FOR
	$(MAKE) -C $(TOOLS)/toolchains/openwatcom compile WORKDIR=$(CURDIR) ARGS='-ml -k16384 -dS2_MSFORTRAN -fe=build/S2F.EXE $(SOURCES)'
clean:
	rm -rf build
