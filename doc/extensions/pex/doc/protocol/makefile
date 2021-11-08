
print:	null
	eqn doc.ms |  psroff $(PRINTER) -i -ms 2> toc.out 

ps:	null
	eqn doc.ms |  psroff -t -i -ms 2> toc.out > doc.ps

toc:	null
	cat toc.hdr > contents
	echo ".Bg 1" >> contents
	echo "PEX Protocol Specification" >> contents
	echo ".Ed 1" >> contents
	cat toc.out >> contents
	psroff $(PRINTER) -ms contents

tocps:	null
	cat toc.hdr > contents
	echo ".Bg 1" >> contents
	echo "PEX Protocol Specification" >> contents
	echo ".Ed 1" >> contents
	cat toc.out >> contents
	psroff -t -ms contents > toc.ps

null:
