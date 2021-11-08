# Copyright 1985, Massachusetts Institute of Technology
# Makefile for the Xlib document.

TROFF= psroff -t
#TROFF= ditroff -Tps
PRINTER= lpr
FILES1= macros.t\
	X11.protocol
FILES2= macros.t\
	X11.keysyms\
	X11.encoding

all: protocol appendix index

protocol: $(FILES1)
	eqn ${FILES1} | tbl |${TROFF} -ms 2> index.raw  > protocol.PS

appendix: $(FILES2)
	tbl ${FILES2} |${TROFF} -ms -n83 2>> index.raw  > appendix.PS

preview:$(FILES1)
	neqn ${FILES1} | tbl | nroff -ms 2> index.nraw | colcrt

index:	index.raw
	sort -f -t: +1 -3 +0n -1n index.raw | awk -f fixindex.awk |awk -f block.awk > index.troff
	cat indexmacros.t index.troff | ${TROFF} -me  > index.PS
	rm -f index.troff

clean:
	rm -f *~ \#* *.bak errs index.traw index.nraw

