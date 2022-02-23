GPP=$(CXX)
CPPFLAGS=-Wall -Wextra -std=c++17 -O3 -g -Ilibsais/src

ODIR=obj
BINDIR=bin
LIBDIR=lib
SRCDIR=src

LIBS=

_DEPS = RankBitvector.h WaveletTree.h FMIndex.h MEMfinder.h ReverseComplementView.h Serialize.h
DEPS = $(patsubst %, $(SRCDIR)/%, $(_DEPS))

_OBJ = RankBitvector.o WaveletTree.o FMIndex.o MEMfinder.o ReverseComplementView.o Serialize.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ)) $(ODIR)/sais.o $(ODIR)/sais64.o

LINKFLAGS = $(CPPFLAGS) -static-libstdc++

VERSION := Branch $(shell git rev-parse --abbrev-ref HEAD) commit $(shell git rev-parse HEAD) $(shell git show -s --format=%ci)

$(shell mkdir -p bin)
$(shell mkdir -p obj)
$(shell mkdir -p lib)

lib: $(LIBDIR)/memfinder.a

$(LIBDIR)/memfinder.a: $(OBJ) $(DEPS)
	ar rvs $@ $(OBJ)

$(BINDIR)/test_wavelet: $(ODIR)/test_wavelet.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_bwt: $(ODIR)/test_bwt.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_fmindex: $(ODIR)/test_fmindex.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_count: $(ODIR)/test_count.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_mems: $(ODIR)/test_mems.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_bestmems: $(ODIR)/test_bestmems.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_mems_bidi: $(ODIR)/test_mems_bidi.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_bestmems_bidi: $(ODIR)/test_bestmems_bidi.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_save: $(ODIR)/test_save.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_mums: $(ODIR)/test_mums.o $(OBJ)
	$(GPP) -o $@ $^

$(ODIR)/sais.o: libsais/src/libsais.c
	$(GPP) -c -o $@ $< $(CPPFLAGS)

$(ODIR)/sais64.o: libsais/src/libsais64.c
	$(GPP) -c -o $@ $< $(CPPFLAGS)

$(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(GPP) -c -o $@ $< $(CPPFLAGS)

all: $(LIBDIR)/memfinder.a $(BINDIR)/test_bwt $(BINDIR)/test_wavelet $(BINDIR)/test_fmindex $(BINDIR)/test_count $(BINDIR)/test_mems $(BINDIR)/test_bestmems $(BINDIR)/test_mems_bidi $(BINDIR)/test_bestmems_bidi $(BINDIR)/test_save $(BINDIR)/test_mums

clean:
	rm -f $(ODIR)/*
	rm -f $(BINDIR)/*
	rm -f $(LIBDIR)/*
