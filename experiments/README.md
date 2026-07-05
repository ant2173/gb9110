# Experiments

The repository keeps selected reproducible milestones rather than every temporary build.

- `gbtable`: first decisive four-pixel renderer improvement;
- `gbrow`: ROW8 renderer plus the first guarded DIV helper;
- `gbfs`: fixed FULL / FS2 / FS3 scheduling with separate guest and display rates;
- `gbdiv2`: layered DIV v2 helper with same-binary OFF / v1 / v2 comparisons.

Intermediate builds — GBHOT, GBTIME, GBFLAGS, GBDIV v1 and the failed GBTILE cache — are documented in [`docs/OPTIMIZATION_HISTORY.md`](../docs/OPTIMIZATION_HISTORY.md).

ROM images and generated `.GEO` binaries are deliberately excluded.
