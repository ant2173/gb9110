#
# GBTABLE v0.6 -- lookup-table renderer benchmark
#

name        gbtable.app
longname    "GB Table Renderer v0.6"

tokenchars  "GTB6"
tokenid     8

type        appl, process, single

class       GBTableProcessClass
appobj      GBTableApp

heapspace   3000

library     geos
library     ui

resource    APPRESOURCE object
resource    INTERFACE object
