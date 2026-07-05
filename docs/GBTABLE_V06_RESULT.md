# GBTABLE v0.6 real-hardware result

Measured on a real Nokia 9110:

```text
1 CORE:          22 guest FPS
2 ORIGINAL PACK:  6 guest FPS
3 TABLE PACK:    12 guest FPS
4 TABLE FULL:     9 guest FPS / 9 blit FPS
5 BLIT 4BPP:     44 blit FPS
```

## Interpretation

The lookup-table renderer is a successful optimization:

- renderer-only throughput doubled;
- complete-frame throughput rose from 5 to 9 FPS;
- the unchanged 44 FPS static blit confirms that the improvement is inside the renderer;
- the next primary bottleneck is the 21–22 FPS CPU/core ceiling.

Approximate frame-time decomposition:

```text
core only:       about 45 ms
table + packing: about 83 ms total
full frame:      about 111 ms total
```

The table renderer therefore adds roughly 38 ms over the core, compared with
roughly 121 ms for the original packed renderer.
