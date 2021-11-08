NROFF = nroff
PTROFF = ptroff
TROFFOPTS = -ms -t

PSFILES = color.PS r3widgets.PS
DOCFILES = color.doc r3widgets.doc

all: ps

ps: $(PSFILES)

doc: $(DOCFILES)

color.PS: color.tbl.ms
	tbl $? | $(PTROFF) $(TROFFOPTS) >$@

color.doc: color.tbl.ms
	tbl $? | $(NROFF) $(TROFFOPTS) >$@

r3widgets.PS: r3widgets.ms
	$(PTROFF) $(TROFFOPTS) <$? >$@

r3widgets.doc: r3widgets.ms
	$(NROFF) $(TROFFOPTS) <$? >$@
