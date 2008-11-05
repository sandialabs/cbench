#!/bin/bash

stagedir=./SF-cbench-staging
sfwebskel=./sfwebskel

mkdir -p $stagedir
rsync -aC $sfwebskel/ $stagedir/

cd $stagedir/htdocs
tar xfz ../../htdocs-common.tar.gz
cd -
cd $stagedir/persistent
tar xfz ../../tracsite.tar.gz
cd -
cd $stagedir
tar xfx ../tracinstall.tar.gz
cd -

chmod -R o+rwx $stagedir/persistent

TBIN=$stagedir/tracinstall/bin
TRACENV=$stagedir/persistent/TRAC

export PYTHONPATH=$stagedir/tracinstall/lib/python2.4/site-packages/:$stagedir/tracinstall/lib64/python2.4/site-packages/

$TBIN/trac-admin $TRACENV upgrade
$TBIN/trac-admin $TRACENV wiki upgrade
/bin/rm -f $stagedir/persistent/TRAC/db/*bak

$TBIN/trac-admin $TRACENV permission remove jbogden TRAC_ADMIN
$TBIN/trac-admin $TRACENV permission remove cdmaest TRAC_ADMIN


$TBIN/trac-admin $TRACENV permission remove authenticated BROWSER_VIEW CHANGESET_VIEW FILE_VIEW LOG_VIEW
$TBIN/trac-admin $TRACENV permission remove authenticated MILESTONE_CREATE MILESTONE_MODIFY MILESTONE_VIEW
$TBIN/trac-admin $TRACENV permission remove authenticated REPORT_CREATE REPORT_MODIFY REPORT_VIEW
$TBIN/trac-admin $TRACENV permission remove authenticated ROADMAP_ADMIN ROADMAP_VIEW SEARCH_VIEW
$TBIN/trac-admin $TRACENV permission remove authenticated TICKET_CREATE TICKET_MODIFY TICKET_VIEW
$TBIN/trac-admin $TRACENV permission remove authenticated TIMELINE_VIEW CONFIG_VIEW
$TBIN/trac-admin $TRACENV permission remove authenticated WIKI_MODIFY WIKI_VIEW WIKI_CREATE


$TBIN/trac-admin $TRACENV permission remove anonymous CHANGESET_VIEW BROWSER_VIEW FILE_VIEW LOG_VIEW
$TBIN/trac-admin $TRACENV permission remove anonymous TICKET_CREATE TICKET_MODIFY
$TBIN/trac-admin $TRACENV permission remove anonymous REPORT_CREATE REPORT_MODIFY
$TBIN/trac-admin $TRACENV permission remove anonymous WIKI_CREATE WIKI_MODIFY
$TBIN/trac-admin $TRACENV permission remove anonymous MILESTONE_CREATE MILESTONE_MODIFY
$TBIN/trac-admin $TRACENV permission remove anonymous ROADMAP_ADMIN CONFIG_VIEW

find SF-cbench-staging -name '*pyc' -exec /bin/rm -f {} \;

patch -p0 < sf/trac-readonly.patch


echo NOTE: rsync -vrptl --delete --exclude persistent $stagedir/ sonicsoft70,cbench@web.sourceforge.net:.
echo NOTE: rsync -vrptl --delete $stagedir/persistent/* sonicsoft70,cbench@web.sourceforge.net:persistent/.
