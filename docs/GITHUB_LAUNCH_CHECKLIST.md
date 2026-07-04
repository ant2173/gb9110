# GitHub launch checklist

## Before making the repository public

- [ ] Replace the placeholder in `LICENSE`
- [ ] Choose repository name (`gb9110` is the current suggestion)
- [ ] Add one clear photo of the app running on the real communicator
- [ ] Verify the clean build instructions from an empty project directory
- [ ] Confirm no ROM, SDK binary, or generated `.GEO` file is tracked
- [ ] Check that every adapted Peanut-GB header retains the MIT notice
- [ ] Review `ACKNOWLEDGEMENTS.md` and `ACKNOWLEDGEMENTS_RU.md`
- [ ] Add the latest lookup-renderer hardware result
- [ ] Decide whether to expose experimental source immediately or on a branch
- [ ] Add a short repository description:
  `Game Boy emulation experiment for the Nokia 9110 Communicator / PC/GEOS`
- [ ] Add topics:
  `nokia-9110`, `pc-geos`, `game-boy`, `emulator`, `retrocomputing`, `borland-cpp`

## Suggested publication order

1. Create the repository quietly.
2. Push the curated baseline, profiler, documentation, and articles.
3. Verify links and build instructions.
4. Publish the first article with a real-device photo.
5. Share it with retrocomputing, GEOS, and emulator-development communities.

Do not lead with “full-speed emulator.” Lead with the true and stronger claim:

> A real Game Boy ROM now executes, renders, and accepts input on a real Nokia 9110, and the bottlenecks have been measured.
