#
# GBPROF v0.3 -- Nokia 9110 performance profiler
#

name        gbprof.app
longname    "GB Profiler v0.3"

tokenchars  "GPR3"
tokenid     8

type        appl, process, single

class       GBProfProcessClass
appobj      GBProfApp

heapspace   3000

library     geos
library     ui

resource    APPRESOURCE object
resource    INTERFACE object
