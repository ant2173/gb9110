# GBHW

First hardware-oriented frontend.

This is the curated baseline that:

- loads `FLAPPY.GB` from `World\ExtrApps`;
- allocates ROM banks and framebuffer through GEOS;
- runs Peanut-GB;
- accepts keyboard input;
- provides selectable 10/30/60 requested modes;
- builds as EC and NC variants.

The real-device build is `GBHW.GEO`.

Current performance is not full-speed. Use `tools/gbprof` for meaningful measurements.
