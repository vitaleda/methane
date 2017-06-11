# smb
Super Methane Bros for AmigaOS 4 / SDL2.

This SDL2 conversion is based on Spot's SDL1 source codes (see OS4Depot.net).

Supports 640x480 (scaled from the original 320x256 size) screen, 2 players and joysticks.

To build, run "gmake" in amigaos4 directory. Command-line parameters:

"window": start in window mode instead of fullscreen.

"nosync": disable VSYNC. If game is too slow, try this.

"sleep": if game runs too fast, sleep a few milliseconds before the next frame (frame cap at 50 Hz)

"filter": smoothen the scale image

Example:

methane window filter
