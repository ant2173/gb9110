#
# GBHW v0.2 -- hardware test build
#

name        gbhw.app
longname    "GB HW Test v0.2"

tokenchars  "GHW2"
tokenid     8

type        appl, process, single

class       GBHWProcessClass
appobj      GBHWApp

heapspace   3000

library     geos
library     ui

resource    APPRESOURCE object
resource    INTERFACE object
