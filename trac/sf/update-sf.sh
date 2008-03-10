#!/bin/bash
set -x

if [ -z $1 ]; then
	ACT="site"
else
	ACT="$1"
fi

PERSIST=/tmp/persistent/cbench
PROJ=/home/groups/c/cb/cbench
HOME=/home/users/s/so/sonicsoft70

if [ "$ACT" == 'site' ]; then
	if [ -d $PERSIST/TRAC.old ]; then
		echo "$PERSIST/TRAC.old exists. Please remove first"
		exit 1
	fi
	
	cd $PROJ/htdocs
	tar xvfz $PROJ/common.tar.gz

	cd $PERSIST
	/bin/mv TRAC TRAC.old
	sync
	tar xvfz $PROJ/tracsite.tar.gz
	chmod -R 0777 $PERSIST
	touch $PERSIST/TRAC/log/trac.log
	chmod 666 $PERSIST/TRAC/log/trac.log

	cd $PROJ
	chmod +rx $PROJ/cgi-bin/trac.cgi

	# this doesn't seem to work anymore on SF servers.... gets hung
	# in diskwait...
	#$PROJ/set_trac_perms.sh

	#$PROJ/tracinstall/bin/trac-admin $PERSIST/TRAC resync
	/bin/rm -f $PROJ/common.tar.gz $PROJ/tracsite.tar.gz

	echo "Please remove $PERSIST/TRAC.old...."
fi

if [ "$ACT" == 'install' ]; then
	find $PROJ -type f -exec chmod 0664 {} \;
	find $PROJ -type d -exec chmod 2775 {} \;
	chmod +rx $PROJ/cgi-bin/trac.cgi
	chmod u+x $PROJ/set_trac_perms.sh $PROJ/update-sf.sh
	
	cd $PROJ
	tar xvfz $PROJ/tracinstall.tar.gz
	#find $PROJ -type f -exec chmod 0664 {} \;
	#find $PROJ -type d -exec chmod 2775 {} \;
	/bin/rm -f $PROJ/tracinstall.tar.gz
fi
