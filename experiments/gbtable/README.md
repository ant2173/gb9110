# GBTABLE

Current experimental renderer.

The design attempts to reduce algorithmic work through:

- four-pixel lookup-table decoding;
- 16-bit packed framebuffer writes;
- precomputed first-ten-sprites-per-line lists;
- per-line sprite filtering.

This experiment has not yet been measured on the real device at the time of this draft.
