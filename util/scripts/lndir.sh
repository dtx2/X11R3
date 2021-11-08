#!/bin/sh

#
# lndir - create shadow link tree
#
# $XConsortium: lndir.sh,v 1.1 88/10/20 17:37:16 jim Exp $
#
# Used to create a copy of the a directory tree that has links for all non-
# directories (except those named RCS).  If you are building the distribution
# on more than one machine, you should use this script.
#
# If your master sources are located in /usr/src/X and you would like
# your link tree to be in /usr/local/src/new-X, do the following:
#
# 	%  mkdir /usr/local/src/new-X
#	%  cd /usr/local/src/new-X
# 	%  lndir /usr/src/X .
#

DIRFROM=$1
DIRTO=$2

pwd=`pwd`

USAGE="Usage: $0 fromdir todir"

case "$DIRFROM" in
/*)	;;
*)	echo "dir \"$DIRFROM\" must begin with a /"
	exit 1
	;;
esac

if [ ! -d $DIRFROM -a ! -d $DIRTO ]
then
	echo "$USAGE"
	exit 1
fi

REALFROM=

DIRLIST=`(cd $DIRFROM; find * \( -type d ! -name 'RCS' \) -print)`

cd $DIRTO

if [ `(cd $DIRFROM; pwd)` = $pwd ]
then
	echo "FROM and TO are identical!"
	exit 1
fi

for dir in $DIRLIST
do
	mkdir $dir
done

for file in `ls $DIRFROM`
do
	ln -s $DIRFROM/$file .
done

for dir in $DIRLIST
do
	echo $dir:
	(cd $dir;
	 pwd=`pwd`
	 if [ `(cd $DIRFROM/$dir; pwd)` = $pwd ]
	 then
		echo "FROM and TO are identical!"
		exit 1
	 fi;
	 for file in `ls $DIRFROM/$dir`
	 do
		ln -s $DIRFROM/$dir/$file .
	 done)
done
