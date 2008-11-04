#!/usr/bin/python -v
# -*- coding: utf-8 -*-
#
# Copyright (C) 2003-2008 Edgewall Software
# Copyright (C) 2003-2004 Jonas Borgström <jonas@edgewall.com>
# All rights reserved.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://trac.edgewall.org/wiki/TracLicense.
#
# This software consists of voluntary contributions made by many
# individuals. For the exact contribution history, see the revision
# history and logs, available at http://trac.edgewall.org/log/.
#
# Author: Jonas Borgström <jonas@edgewall.com>

import sys
import os

#print os.getenv('PYTHONPATH')
#print '    '

#sys.path.append('/home/groups/c/cb/cbench/tracinstall/lib/python2.4/site-packages/')

sys.path.insert(0,'/home/groups/c/cb/cbench/tracinstall/lib/python2.4/site-packages/')
sys.path.insert(0,'/home/groups/c/cb/cbench/tracinstall/lib64/python2.4/site-packages/')
sys.path.insert(0,'/home/groups/c/cb/cbench/tracinstall/usr/lib/python2.4/site-packages/')

#sys.path.insert(0,'/home/groups/c/cb/cbench/tracinstall/lib/python2.4/site-packages/Trac-0.11.1-py2.4.egg')
#sys.path.insert(0,'/home/groups/c/cb/cbench/tracinstall/lib/python2.4/site-packages/Genshi-0.5.1-py2.4-linux-x86_64.egg')

#print 'Status: 500 Internal Server Error'
#print 'Content-type: text/plain'
#print os.getenv('PYTHONPATH')
#print '    '

os.environ['TRAC_ENV'] = "/home/groups/c/cb/cbench/persistent/TRAC"
os.environ['PYTHON_EGG_CACHE'] = "/home/groups/c/cb/cbench/persistent/egg-cache"

try:
    import pkg_resources
#    if 'PYTHON_EGG_CACHE' not in os.environ:
#        if 'TRAC_ENV' in os.environ:
#            egg_cache = os.path.join(os.environ['TRAC_ENV'], '.egg-cache')
#        pkg_resources.set_extraction_path(egg_cache)
    from trac.web import cgi_frontend
    cgi_frontend.run()
except SystemExit:
    raise
except Exception, e:
    import traceback
    import os

    print>>sys.stderr, e
    traceback.print_exc(file=sys.stderr)

    print 'Status: 500 Internal Server Error'
    print 'Content-Type: text/plain'
    print
    print 'Oops...'
    print
    print 'Trac detected an internal error:', e
    print
    traceback.print_exc(file=sys.stdout)
    print
    print os.getenv('PYTHONPATH')
