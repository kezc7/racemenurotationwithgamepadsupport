# Racemenu Rotation with Gamepad Support NG

A fork of [Racemenu Rotation with Gamepad Support](https://www.nexusmods.com/skyrimspecialedition/mods/XXXXX) 
by [Thewyrmking95](https://github.com/Thewyrmking95), which is itself a fork of 
[Another RaceMenu Rotation Mod](https://www.nexusmods.com/skyrimspecialedition/mods/XXXXX) 
by DarkMatterValkyrie.

## Why This Exists

While investigating game crashes, a crash log pointed to `PlayerRotationGPSupport.dll` 
running code outside of RaceMenu during normal gameplay — something it should never do.

Looking at the source code revealed 5 bugs, including one that caused crashes by doing 
an unnecessary and unsafe scene graph traversal every frame. This fork fixes all of them.

## Bugs Fixed

1. **Game crash** — Redundant `UpdateWorldData` call corrupted game memory every frame
2. **Gamepad rotation stuck on** — `allow_rotate` was never reset when releasing the thumbstick
3. **Rotation persisting after menu close** — State was never cleaned up when RaceMenu closed
4. **Gamepad rotation never worked** — Thumbstick input was stored in the wrong variable
5. **Missing null check** — Player reference was not checked before use on menu open

## Credits

- [DarkMatterValkyrie](https://www.nexusmods.com/skyrimspecialedition/users/XXXXX) — Original mod
- [Thewyrmking95](https://github.com/Thewyrmking95) — Gamepad fork
- Bug fixes by [kezc](https://www.nexusmods.com/profile/kezc)


Original
I was annoyed that none of the new racemenu rotation mods worked like the old one that never got updated. The closest by far was "Another RaceMenu Rotation Mod" which allowed smooth rotation with right click but had no gamepad support.
(Yes, I do play skyrim with a controller. Deal with it.) So, I saw the creator of that mod had linked their source files and I wanted the thumbstick rotation in the game like the old mod so I added it.
I did not recieve permission from the mod author and this is my first release so if asked I will remove it but, hopefully in the event that happens they will add my changes to the existing mod.
