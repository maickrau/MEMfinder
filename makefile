GPP=$(CXX)
CPPFLAGS=-Wall -Wextra -std=c++17 -O3 -g -Ilibsais/src

ODIR=obj
BINDIR=bin
SRCDIR=src

LIBS=

_DEPS =
DEPS = $(patsubst %, $(SRCDIR)/%, $(_DEPS))

_OBJ =
OBJ = $(patsubst %, $(ODIR)/%, $(_OBJ))

LINKFLAGS = $(CPPFLAGS) -static-libstdc++

VERSION := Branch $(shell git rev-parse --abbrev-ref HEAD) commit $(shell git rev-parse HEAD) $(shell git show -s --format=%ci)

$(shell mkdir -p bin)
$(shell mkdir -p obj)

$(BINDIR)/test_bwt: $(ODIR)/test_bwt.o libsais/src/libsais64.c libsais/src/libsais.c
	$(GPP) -o $@ $^

$(ODIR)/test_bwt.o: $(SRCDIR)/test_bwt.cpp $(DEPS)
	$(GPP) -c -o $@ $< $(CPPFLAGS)

$(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(GPP) -c -o $@ $< $(CPPFLAGS)

all: $(BINDIR)/test_bwt

clean:
	rm -f $(ODIR)/*
	rm -f $(BINDIR)/*
