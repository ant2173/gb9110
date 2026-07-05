#
# GBPROF v1.5j -- opcode and data-memory profiler
# Unique filename and token prevent PC/GEOS from launching GBROW/GBFS.
#

name        gbprof.app
longname    "GB Profiler v1.5j"

tokenchars  "GBP1"
tokenid     8

type        appl, process, single

class       GBRowProcessClass
appobj      GBRowApp

heapspace   3500

library     geos
library     ui

resource    APPRESOURCE object
resource    INTERFACE object
