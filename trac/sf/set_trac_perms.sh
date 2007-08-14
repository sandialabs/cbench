#!/bin/bash

TBIN=/home/users/s/so/sonicsoft70/cbench-sf/tracinstall/bin
TRACENV=/tmp/persistent/cbench-sf/TRAC

$TBIN/trac-admin $TRACENV permission remove jbogden TRAC_ADMIN


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

