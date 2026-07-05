# GBTABLE v0.6

Historical milestone: the first renderer rewrite with a decisive real-hardware gain.

```text
Original packed renderer   6 FPS
GBTABLE packed renderer   12 FPS
Original full path         5 FPS
GBTABLE full path          9 FPS
Static 4-bpp blit         44 FPS
```

The build uses four-pixel lookup-table groups and prepared sprite lists.

The ROM is not included.
