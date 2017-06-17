VOX8 ~ by Voxel9
================

VOX8 is a heavy WIP multiplatform Chip8 emulator which was branched off of my recent Chip8 disassembler. Please expect bugs, as some ROMs experience weird behaviours, but it does run as particularly well as most existing Chip8 emulators.
At the moment, only PS Vita and 3DS builds are available, but I will put together an OpenGL-based build for PC very soon.

Building
========
`PS Vita` ~ Ensure you have GCC/DevkitPro/Msys/Whatever installed, as well as VitaSDK (https://vitasdk.org). Also make sure you have installed libvita2d as well (https://github.com/xerpi/libvita2d).
In CMD/Terminal, 'cd' to the folder with Makefile inside it and type `make`. This should output a VPK which you can then install via VitaShell.

`3DS` ~ Ensure you have DevkitPro installed, and update your ctrulib and libcitro3d installations. (https://github.com/smealum/ctrulib and https://github.com/fincs/citro3d respectively) Also install xerpi's libsf2d (https://github.com/xerpi/sf2dlib).
In CMD/Terminal, 'cd' to the folder with Makefile inside it and type `make`. This should output a 3dsx file and an smdh file which you can place at _sdmc:/3ds/VOX8/_.

Usage
=====
`PS Vita` ~ For now it's a bit fiddly. Place the ROM you want to run at ux0:/data/VOX8/ and rename it to "ROM" (no quotes). Every time you want to run a different ROM, you have to rename it to "ROM" as well, and replace the old ROM with that one.

`3DS` ~ Place the ROM you want to run in the same directory as the 3dsx file, and rename it to "ROM" (no quotes). Every time you want to run a different ROM, you have to rename it to "ROM" as well, and replace the old ROM with that one.

TODO
====
- Fix weird stuff that happens in some ROMs (probably maths-related errors)
- Add a ROM selection menu.