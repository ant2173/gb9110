#
# GBDIV2 v1.6 -- DIV v1 versus DIV v2 benchmark
# Unique filename and token prevent PC/GEOS from launching older builds.
#

name        gbdiv2.app
longname    "GB DIV2 Bench v1.6"

tokenchars  "GBD2"
tokenid     8

type        appl, process, single

class       GBRowProcessClass
appobj      GBRowApp

heapspace   3200

library     geos
library     ui

resource    APPRESOURCE object
resource    INTERFACE object
