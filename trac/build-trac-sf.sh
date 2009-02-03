#!/bin/bash


TRACURL=http://ftp.edgewall.com/pub/trac
TRAC=trac-0.10.4
CLEARURL=http://www.clearsilver.net/downloads
CLEAR=clearsilver-0.10.4
SQLITEURL=http://www.sqlite.org
#SQLITE=sqlite-3.3.17
SQLITE=sqlite-3.5.6
#PYSQLITEURL=http://initd.org/pub/software/pysqlite/releases/2.3/2.3.5
PYSQLITEURL=http://oss.itsystementwicklung.de/download/pysqlite/2.3/2.3.5
PYSQLITE=pysqlite-2.3.5

#INSTALLROOT=/home/users/s/so/sonicsoft70/tracinstall
#INSTALLROOT=/home/groups/c/cb/cbench-sf/tracinstall
INSTALLROOT=/home/groups/c/cb/cbench/tracinstall
BUILDROOT=/tmp/tracbuild

rm -rf $INSTALLROOT
mkdir -p $INSTALLROOT

if [ "$1" == 'realclean' ]; then
	rm -rf $BUILDROOT
fi

mkdir -p $BUILDROOT
pushd $BUILDROOT

if [ "$1" == 'clean' ]; then
	rm -rf  $CLEAR $TRAC $SQLITE $PYSQLITE
fi

if [ ! -e $CLEAR.tar.gz ]; then
	wget $CLEARURL/$CLEAR.tar.gz
fi
[ -f $CLEAR.tar.gz ] || exit
tar xfz $CLEAR.tar.gz
cd $CLEAR
export PYTHON=/usr/bin/python
#./configure --prefix=$INSTALLROOT/usr --enable-apache --enable-python --enable-perl --disable-java
./configure --prefix=$INSTALLROOT --enable-apache --enable-python --disable-perl --disable-java
make
make install
#make install \
#	DESTDIR="$INSTALLROOT" \
#	INSTALLDIRS="vendor" 
make -C python install  \
	DESTDIR="$INSTALLROOT" \
	INSTALLDIRS="vendor" 
#make install -C perl \
##	DESTDIR="$INSTALLROOT" \
#	INSTALLDIRS="vendor" 

cd -


if [ ! -e $SQLITE.tar.gz ]; then
	wget $SQLITEURL/$SQLITE.tar.gz
fi
[ -f $SQLITE.tar.gz ] || exit
tar xfz $SQLITE.tar.gz
mkdir -p $SQLITE/tmp-build
cd $SQLITE/tmp-build
export CFLAGS="$CFLAGS -DNDEBUG=1"
../configure --prefix=$INSTALLROOT --enable-utf8 --disable-tcl  --enable-shared=no
make
make install
cd -


if [ ! -e $PYSQLITE.tar.gz ]; then
	wget $PYSQLITEURL/$PYSQLITE.tar.gz
fi
[ -f $PYSQLITE.tar.gz ] || exit
tar xfz $PYSQLITE.tar.gz
cd $PYSQLITE
#cp ../setuptools-0.6a9-py2.3.egg .

cat << EOF > setup.cfg
[build_ext]
define=
include_dirs=$INSTALLROOT/include
library_dirs=$INSTALLROOT/lib
libraries=sqlite3
EOF

python ./setup.py build
python ./setup.py install -O1 --skip-build  --prefix=$INSTALLROOT
cd -

if [ ! -e $TRAC.tar.gz ]; then
	wget $TRACURL/$TRAC.tar.gz
fi
tar xfz $TRAC.tar.gz
cd $TRAC
#python ./setup.py install --root=$INSTALLROOT
python ./setup.py install --prefix=$INSTALLROOT
cd -

popd

tar -C /home/groups/c/cb/cbench  --exclude pysqlite2-doc -z -cvf tracinstall.tar.gz ./tracinstall