# PlayerRotationGPSupport 1.3.0

## Highlights

- Fixed unsafe rotation work during Skyrim save/load, revert, form
  initialization, world transitions, and player 3D reconstruction.
- Reset temporary input and rotation state across load, new-game, save, and
  RaceMenu transitions, including loading multiple saves in one process.
- Reacquired the player and 3D root on each usable frame; no scene-graph
  pointer is retained across frames or loads.
- Replaced unsafe input-event casts with CommonLibSSE-NG type-safe accessors.
- Preserved mouse, configurable keyboard/mouse hold behavior, right-thumbstick
  rotation, and existing configuration keys.
- Added relocation validation and fail-closed behavior for unsupported runtimes.

## Compatibility

- Skyrim SE/AE 1.6.1170
- Skyrim 1.7.104
- Matching SKSE64 and Address Library for SKSE Plugins required
- Skyrim VR unsupported

Existing users can install this release over 1.2.0 without starting a new
game, cleaning saves, or removing unrelated mods. The plugin has no persistent
save or SKSE co-save data.

The lifecycle investigation found unsafe exposure in the plugin, but the
original crash stack also included another native plugin; the exact sole root
cause was not proven.
