ps:	null
	cat doc.ms | refer -e -s -p pex.bib | eqn | tbl | pic -Tpsc | \
		psroff -t -i -ms 2> toc.out > doc.ps

tocps:	null
	cat toc.hdr > contents
	echo ".Bg 1" >> contents
	echo "PEX Introduction and Overview" >> contents
	echo ".Ed 1" >> contents
	cat toc.out >> contents
	psroff -t -ms contents > toc.ps

null:
