:

#
# script to generate a Makefile from an Imakefile.  Sometimes useful for 
# generating Makefiles for stuff outside the X sources.
# 

progname=$0
xtop=$1
what=$2

if [ x"$xtop" = x ]; then
    echo "usage:  $progname  xsourcespath  [whattomake]"
    exit 1
fi

if [ x"$what" = x ]; then
    what="Makefile"
fi

if [ ! -d $xtop ]; then
    echo "$progname"":  no such directory $xtop"
    exit 1
fi

if [ ! -d $xtop/util -o ! -d $xtop/util/imake.includes ]; then
    echo "$progname"":  no X configuration files under $xtop"
    exit 1
fi

if [ ! -f Imakefile ]; then
    echo "$progname"":  can't find `pwd`/Imakefile"
    exit 1
fi

echo "Making $what from Imakefile"
PATH=$xtop/util/imake:$PATH \
imake -DTOPDIR=$xtop -TImake.tmpl -I$xtop/util/imake.includes -s Makefile $what


