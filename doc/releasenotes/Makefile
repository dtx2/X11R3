# Copyright 1985, 1986, 1987, 1988 Massachusetts Institute of Technology
# Makefile for the Installation document.

TROFF= ptroff -t
FILES= 	rn-1.ms rn-2.ms rn-3.ms rn-4.ms rn-5.ms rn-6.ms 

all: relnotes.PS

relnotes.lpt:  $(FILES)
	neqn ${FILES} | tbl | nroff -ms >$@

relnotes.PS:  $(FILES)
	eqn $(FILES) | tbl | $(TROFF) -ms >$@	

relnotes.txt:  $(FILES)
	neqn ${FILES} | tbl | nroff -ms | colcrt >$@

clean:
	rm -f *~ \#* *.bak errs 
