#Gnu makefile for gnu+Linux system

CC=g++

IDIR =include
ODIR=obj
SRCDIR=src
OUTDIR=bin
OUTNAME=blobs_linux

CXXFLAGS=-std=c++17 -Wall -O3 -Wextra -Wpedantic -Wdouble-promotion -I$(IDIR)

LIBS =





_OBJ = main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

default: population.tsv  demographics.tsv
	gnuplot plot.gpi

demographics.tsv population.tsv: $(OUTDIR)/$(OUTNAME)
	$(OUTDIR)/$(OUTNAME) 32 4 2.0 1.75 0.5 1.5 0.75 2> population.tsv 1> demographics.tsv

#The main file depends on EVERYTHING
$(ODIR)/main.o: $(SRCDIR)/main.cpp
	$(CC) -c -o $@ $< $(CXXFLAGS)


#Compile the final program
$(OUTDIR)/$(OUTNAME):	$(OBJ)
	$(CC) -o $@ $^ $(CXXFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f $(OUTDIR)/$(OUTNAME)
