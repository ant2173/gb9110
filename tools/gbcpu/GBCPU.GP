#
# GBCPU v0.7 -- LR35902 opcode and memory profiler
#

name        gbcpu.app
longname    "GB CPU Profiler v0.7"

tokenchars  "GCP7"
tokenid     8

type        appl, process, single

class       GBCPUProcessClass
appobj      GBCPUApp

heapspace   3000

library     geos
library     ui

resource    APPRESOURCE object
resource    INTERFACE object
