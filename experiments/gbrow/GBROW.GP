#
# GBROW v1.3 -- eight-pixel background renderer benchmark
#

name        gbrow.app
longname    "GB Row8 Renderer v1.3"

tokenchars  "GRW1"
tokenid     8

type        appl, process, single

class       GBRowProcessClass
appobj      GBRowApp

heapspace   3000

library     geos
library     ui

resource    APPRESOURCE object
resource    INTERFACE object
