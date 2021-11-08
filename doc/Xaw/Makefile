# Copyright 1985, Massachusetts Institute of Technology
# Makefile for the Toolkit documents.

TROFF= psroff -t
#TROFF= ditroff -Tps
NROFF= nroff
PRINTER= lpr

WIDGETSRC= \
	macros.t \
	strings.mit \
	Xtk.widg.front \
	Xtk.widgets

all: ps

ps: widgets.PS

text: widgets.txt

widgets: widgets.txt widgets.PS

widgets.PS: $(WIDGETSRC)
	rm -f index.traw widgets.PS index.PS
	tbl $(WIDGETSRC) | \
		${TROFF} -ms 2> index.traw  > widgets.PS
	make index.PS

widgets.txt: $(WIDGETSRC)
	rm -f index.nraw widgets.txt index.txt
	tbl $(WIDGETSRC) | neqn | \
		${NROFF} -ms 2> index.nraw  > widgets.txt
	make index.txt

index.PS: index.traw
	rm -f index.PS
	sort -f -t: +1 -3 +0n -1n index.traw | awk -f fixindex.awk \
		| awk -f block.awk > index.troff
	cat widg.idxmac.t index.troff | ${TROFF} -me  > \
		index.PS
	rm -f index.troff index.traw

index.txt: index.nraw
	rm -f index.txt
	sort -f -t: +1 -3 +0n -1n index.nraw | awk -f fixindex.awk \
		| awk -f block.awk > index.nroff
	cat widg.idxmac.t index.nroff | ${NROFF} -me > \
		index.txt
	rm -f index.nroff index.nraw

clean:
	rm -f *~ \#* *.bak errs index.?raw *.?roff
