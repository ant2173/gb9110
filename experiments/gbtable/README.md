# GBTABLE

Current experimental renderer.

The design attempts to reduce algorithmic work through:

- four-pixel lookup-table decoding;
- 16-bit packed framebuffer writes;
- precomputed first-ten-sprites-per-line lists;
- per-line sprite filtering.

Measured on the real device:

- core only: 22 guest FPS;
- original packed renderer: 6 guest FPS;
- table packed renderer: 12 guest FPS;
- table renderer with GEOS blit: 9 guest FPS;
- static 4-bpp blit: 44 FPS.
