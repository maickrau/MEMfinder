GPP=$(CXX)
CPPFLAGS=-Wall -Wextra -std=c++17 -O3 -g -Ilibsais/src

ODIR=obj
BINDIR=bin
SRCDIR=src

LIBS=

_DEPS = RankBitvector.h WaveletTree.h FMIndex.h MEMfinder.h ReverseComplementView.h
DEPS = $(patsubst %, $(SRCDIR)/%, $(_DEPS))

_OBJ = RankBitvector.o WaveletTree.o FMIndex.o MEMfinder.o ReverseComplementView.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

LINKFLAGS = $(CPPFLAGS) -static-libstdc++

VERSION := Branch $(shell git rev-parse --abbrev-ref HEAD) commit $(shell git rev-parse HEAD) $(shell git show -s --format=%ci)

$(shell mkdir -p bin)
$(shell mkdir -p obj)

$(BINDIR)/test_wavelet: $(ODIR)/test_wavelet.o $(OBJ) libsais/src/libsais64.c libsais/src/libsais.c
	$(GPP) -o $@ $^

$(BINDIR)/test_bwt: $(ODIR)/test_bwt.o $(OBJ) libsais/src/libsais64.c libsais/src/libsais.c
	$(GPP) -o $@ $^

$(BINDIR)/test_fmindex: $(ODIR)/test_fmindex.o $(OBJ) libsais/src/libsais64.c libsais/src/libsais.c
	$(GPP) -o $@ $^

$(BINDIR)/test_count: $(ODIR)/test_count.o $(OBJ) libsais/src/libsais64.c libsais/src/libsais.c
	$(GPP) -o $@ $^

$(BINDIR)/test_mems: $(ODIR)/test_mems.o $(OBJ) libsais/src/libsais64.c libsais/src/libsais.c
	$(GPP) -o $@ $^

$(BINDIR)/test_bestmems: $(ODIR)/test_bestmems.o $(OBJ) libsais/src/libsais64.c libsais/src/libsais.c
	$(GPP) -o $@ $^

$(BINDIR)/test_mems_bidi: $(ODIR)/test_mems_bidi.o $(OBJ) libsais/src/libsais64.c libsais/src/libsais.c
	$(GPP) -o $@ $^

$(BINDIR)/test_bestmems_bidi: $(ODIR)/test_bestmems_bidi.o $(OBJ) libsais/src/libsais64.c libsais/src/libsais.c
	$(GPP) -o $@ $^

$(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(GPP) -c -o $@ $< $(CPPFLAGS)

all: $(BINDIR)/test_bwt $(BINDIR)/test_wavelet $(BINDIR)/test_fmindex $(BINDIR)/test_count $(BINDIR)/test_mems $(BINDIR)/test_bestmems $(BINDIR)/test_mems_bidi $(BINDIR)/test_bestmems_bidi

clean:
	rm -f $(ODIR)/*
	rm -f $(BINDIR)/*
