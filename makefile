GPP=$(CXX)
CPPFLAGS=-Wall -Wextra -std=c++17 -O3 -g -Ilibsais/src -IPartSortBWT/src $(DEBUGFLAG)

ODIR=obj
BINDIR=bin
LIBDIR=lib
SRCDIR=src

LIBS=

_DEPS = RankBitvector.h WaveletTree.h FMIndex.h MEMfinder.h ReverseComplementView.h Serialize.h FlatRanks.h
DEPS = $(patsubst %, $(SRCDIR)/%, $(_DEPS))

_OBJ = RankBitvector.o WaveletTree.o FMIndex.o MEMfinder.o ReverseComplementView.o Serialize.o FlatRanks.o
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ)) $(ODIR)/sais.o $(ODIR)/sais64.o $(ODIR)/PartSortBWT.o

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

$(BINDIR)/test_mems_prefixindex: $(ODIR)/test_mems_prefixindex.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_bestmems_bidi: $(ODIR)/test_bestmems_bidi.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_save: $(ODIR)/test_save.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_mums: $(ODIR)/test_mums.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_mem_weighted: $(ODIR)/test_mem_weighted.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_bwt_partsort: $(ODIR)/test_bwt_partsort.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_fmindex_lowmemory: $(ODIR)/test_fmindex_lowmemory.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_fmindex_memory_match: $(ODIR)/test_fmindex_memory_match.o $(OBJ)
	$(GPP) -o $@ $^

$(BINDIR)/test_bigrank: $(ODIR)/test_bigrank.o $(OBJ)
	$(GPP) -o $@ $^

$(ODIR)/PartSortBWT.o: PartSortBWT/src/PartSortBWT.cpp
	$(GPP) -c -o $@ $< $(CPPFLAGS)

$(ODIR)/sais.o: libsais/src/libsais.c
	$(GPP) -c -o $@ $< $(CPPFLAGS)

$(ODIR)/sais64.o: libsais/src/libsais64.c
	$(GPP) -c -o $@ $< $(CPPFLAGS)

$(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(GPP) -c -o $@ $< $(CPPFLAGS)

all: $(LIBDIR)/memfinder.a $(BINDIR)/test_bwt $(BINDIR)/test_wavelet $(BINDIR)/test_fmindex $(BINDIR)/test_count $(BINDIR)/test_mems $(BINDIR)/test_bestmems $(BINDIR)/test_mems_bidi $(BINDIR)/test_bestmems_bidi $(BINDIR)/test_save $(BINDIR)/test_mums $(BINDIR)/test_mem_weighted $(BINDIR)/test_bwt_partsort $(BINDIR)/test_fmindex_lowmemory $(BINDIR)/test_fmindex_memory_match $(BINDIR)/test_mems_prefixindex $(BINDIR)/test_bigrank

clean:
	rm -f $(ODIR)/*
	rm -f $(BINDIR)/*
	rm -f $(LIBDIR)/*
