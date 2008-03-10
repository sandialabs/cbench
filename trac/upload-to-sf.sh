#!/bin/bash
set -x

# build a tarball of Cbench TRAC site
SRC=/data/cbench/cbench
TMP=/tmp/tracsite
SFSTUFF=$HOME/tracstuff/sf
SFPROJ=cbench

mkdir -p $TMP
rm -rf $TMP/*

# create empty SVN
#svnadmin create $TMP/SVN
mkdir -p $TMP/SVN
#rsync -a --exclude mylogs --exclude hooks --exclude dav $SRC/SVNnew/ $TMP/SVN/.

# build a TRAC site image for sf
rsync --exclude trac.db* --exclude trac.log -a $SRC/TRAC $TMP/.
/bin/rm -f $TMP/TRAC/plugins/TracWebAdmin* $TMP/TRAC/plugins/TracAccountManager* $TMP/TRAC/plugins/graphviz*
/bin/rm -rf $TMP/TRAC/.python-eggs
sqlite $SRC/TRAC/db/trac.db .dump | sqlite3 $TMP/TRAC/db/trac.db
#/home/groups/c/cb/cbench-sf/tracinstall/bin/trac-admin $TMP/TRAC upgrade

cp $SFSTUFF/trac.ini $TMP/TRAC/conf/.

cd $TMP
tar cvfz $TMP/tracsite.tar.gz TRAC SVN
cd -

cd $HOME/htdocs
tar cvfz  $TMP/common.tar.gz common
cd -

# copy TRAC site image to sf
$HOME/sf/scp_to_shell $TMP/tracsite.tar.gz
$HOME/sf/scp_to_shell $TMP/common.tar.gz

# copy TRAC install to sf
$HOME/sf/scp_to_shell $HOME/tracinstall.tar.gz

$HOME/sf/scp_to_shell $SFSTUFF/update-sf.sh .
$HOME/sf/scp_to_shell $SFSTUFF/set_trac_perms.sh .
$HOME/sf/scp_to_shell $SFSTUFF/ls2html.sh .
$HOME/sf/login_to_shell "chmod ug+x $SFPROJ/set_trac_perms.sh $SFPROJ/update-sf.sh $SFPROJ/ls2html.sh"
