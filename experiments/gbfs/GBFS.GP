#
# GBFS v1.4c -- isolated frame-skip test
# Unique filename and token prevent PC/GEOS from launching old GBROW/GBSKIP.
#

name        gbfs.app
longname    "GB FS Test v1.4c"

tokenchars  "GBF1"
tokenid     8

type        appl, process, single

class       GBRowProcessClass
appobj      GBRowApp

heapspace   3000

library     geos
library     ui

resource    APPRESOURCE object
resource    INTERFACE object
