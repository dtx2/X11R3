# Copyright 1985, Massachusetts Institute of Technology
# Makefile for the Xlib document.

TROFF= psroff -t
#TROFF= ditroff -Tpsc
PRINTER= lpr
FILES= macros.t\
	abstract.t\
	credits.t\
	CH[01][0123456789]\
	appendixA\
	appendixB\
	appendixC\
	appendixD\
	glossary\
	postproc

XLIBDOC = 
XLIBMAN = Xman
SUBDIRS = \
	$(XLIBMAN)

all: xlib index

xlib: $(FILES)
	eqn ${FILES} | tbl |${TROFF} -ms 2> index.raw  > xlib.PS

preview:$(FILES)
	neqn ${FILES} | tbl | nroff -ms 2> index.nraw | colcrt

index.raw:
	eqn ${FILES} | tbl | ${TROFF} -ms > /dev/null 2> index.raw|\
	${PRINTER}

index:	index.raw
	sort -f -t: +1 -3 +0n -1n index.raw | awk -f fixindex.awk |awk -f block.awk > index.troff
	cat indexmacros.t index.troff | ${TROFF} -me > index.PS
	rm -f index.troff

install:
	@case '${MFLAGS}' in *[ik]*) set +e;; esac; \
	for i in $(SUBDIRS) ;\
	do \
		(cd $$i ; echo "installing $$i"; \
			$(MAKE) $(MFLAGS) \
				INSTALL='$(INSTALL)' \
				DESTDIR='$(DESTDIR)' install ); \
	done

clean:
	rm -f *~ \#* *.bak errs index.traw index.nraw
