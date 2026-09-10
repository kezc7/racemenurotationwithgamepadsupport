# RaceMenu Rotation with Gamepad Support NG

## Version 1.3.0

RaceMenu player rotation with mouse/keyboard and right-thumbstick support.
This release uses one CommonLibSSE-NG-compatible DLL for the supported flat
Skyrim runtimes:

- Skyrim SE/AE 1.6.1170
- Skyrim 1.7.104

Requirements:

- SKSE64 matching the installed Skyrim runtime
- Address Library for SKSE Plugins matching the installed runtime
- RaceMenu

This plugin is not compatible with Skyrim VR.

## Installation

Install the release ZIP through Mod Organizer 2 or Vortex. Its contents are
already relative to the Skyrim `Data` directory:

```text
SKSE/Plugins/PlayerRotationGPSupport.dll
SKSE/Plugins/PlayerRotation.toml
```

Existing users can upgrade in place. No new game, save cleaning, or removal of
unrelated mods is required. The plugin has no Papyrus scripts, ESP, persistent
save data, or SKSE co-save data.

## Configuration

The optional configuration file is `Data/SKSE/Plugins/PlayerRotation.toml`:

```toml
KeyCode = 257
MinimumRotationSpeed = 3.0
MaximumRotationSpeed = 12.0
```

`KeyCode` uses SKSE input codes: keyboard keys are DirectX scancodes 0–255;
mouse buttons start at 256 (`257` is the right mouse button). The configured
keyboard/mouse button must be held while moving the mouse. Gamepad rotation
uses the right thumbstick and does not depend on `KeyCode`.

## 1.3.0 crash and lifecycle fix

The plugin now disables rotation work during Skyrim save/load/revert
transitions, form initialization, player positioning, quit/reset transitions,
and player 3D reconstruction. It clears temporary input and rotation state on
load, new-game, save, and RaceMenu lifecycle events, waits for post-load/new-
game settling, and reacquires the player 3D root instead of retaining scene
graph pointers across frames or loads.

Input handling now uses CommonLibSSE-NG type-safe event accessors. Relocations
are validated before the Main update hook is installed; unsupported runtimes
fail closed instead of patching an invalid address.

The crash investigation identified unsafe lifecycle exposure in the plugin,
but the supplied crash stack also contained another native plugin, so this
release does not claim that PlayerRotationGPSupport was conclusively the only
faulting component.

## Credits

- DarkMatterValkyrie — original RaceMenu rotation mod
- Thewyrmking95 — gamepad fork
- kezc7 — NG port and fixes
