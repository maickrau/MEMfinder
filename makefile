GPP=$(CXX)
CPPFLAGS=-Wall -Wextra -std=c++17 -O3 -g -Ilibsais/src

ODIR=obj
BINDIR=bin
SRCDIR=src

LIBS=

_DEPS = RankBitvector.h WaveletTree.h
DEPS = $(patsubst %, $(SRCDIR)/%, $(_DEPS))

_OBJ = RankBitvector.o WaveletTree.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

LINKFLAGS = $(CPPFLAGS) -static-libstdc++

VERSION := Branch $(shell git rev-parse --abbrev-ref HEAD) commit $(shell git rev-parse HEAD) $(shell git show -s --format=%ci)

$(shell mkdir -p bin)
$(shell mkdir -p obj)

$(BINDIR)/test_wavelet: $(ODIR)/test_wavelet.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_bwt: $(ODIR)/test_bwt.o libsais/src/libsais64.c libsais/src/libsais.c
	$(GPP) -o $@ $^

$(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(GPP) -c -o $@ $< $(CPPFLAGS)

all: $(BINDIR)/test_bwt $(BINDIR)/test_wavelet

clean:
	rm -f $(ODIR)/*
	rm -f $(BINDIR)/*
