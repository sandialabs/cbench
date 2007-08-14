#!/bin/bash

trac-admin /home/cbench/TRAC permission add $1 LOG_VIEW FILE_VIEW CHANGESET_VIEW BROWSER_VIEW
trac-admin /home/cbench/TRAC permission add $1 TICKET_VIEW TICKET_CREATE TICKET_MODIFY
trac-admin /home/cbench/TRAC permission add $1 REPORT_VIEW REPORT_CREATE REPORT_MODIFY
trac-admin /home/cbench/TRAC permission add $1 WIKI_VIEW WIKI_CREATE WIKI_MODIFY
trac-admin /home/cbench/TRAC permission add $1 MILESTONE_VIEW MILESTONE_CREATE MILESTONE_MODIFY
trac-admin /home/cbench/TRAC permission add $1 ROADMAP_VIEW ROADMAP_ADMIN TIMELINE_VIEW SEARCH_VIEW

