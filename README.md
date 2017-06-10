# smb
Super Methane Bros for AmigaOS 4 / SDL2.

This SDL2 conversion is based on Spot's SDL1 source codes (OS4Depot.net).

Supports 640*480 (scaled from 320*240) screen, 2 players and joysticks.

To build, run "gmake" in amigaos4 directory. Command-line parameters:

window: start in window mode instead of fullscreen.
nosync: disable VSYNC. If game is too slow, try this.
sleep: if game runs too fast, sleep before the next frame (50Hz)
filter: smoothen the scale image