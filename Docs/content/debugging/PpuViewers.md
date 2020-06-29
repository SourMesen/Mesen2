---
title: PPU Viewers
weight: 20
pre: ""
chapter: false
---

The PPU viewers are a collection of separate tools that allow you to view the current state of various parts of the PPU's memory - tiles, tilemaps, sprites and palettes.

All viewers have some settings in common:

* **Auto-refresh**: Enabled by default, this makes the viewer refresh automatically at a specific scanline/cycle each frame.
* **Auto-refresh speed**: Configures at what speed the viewer will refresh itself (15/30/60 FPS)
* **Refresh on scanline [y] and cycle [x]**: When using the auto-refresh option, this allows you to control at which point in a frame (cycle & scanline) the data is refreshed while the emulation is running.

You can **zoom in and out** the viewer by using the zoom in and zoom out shortcuts in the view menu, or by holding the `Ctrl` key and using the mouse wheel.
Additionally, you can pan/scroll the viewer by clicking and dragging, or by using the mouse wheel (use the `Shift` key to scroll horizontally.)

## Tilemap Viewer ##

<div class="imgBox"><div>
	<img src="/images/TilemapViewer.png" />
	<span>Tilemap Viewer</span>
</div></div>

The tilemap viewer displays the contents of each background layer, based on the current PPU mode.
For Game Boy games, this displays either the background or window's contents.

<kbd>**Click**</kbd> on a tile in the tilemap to display information about the selected tile on the right. The information displayed changes depending on whether the current ROM is a SNES or Game Boy game.

There are also a number of display options:

* **Show tile grid**: Displays a 8x8 grid (or 16x16 depending on PPU settings) over the image to help distinguish each tile.
* **Show scroll Overlay**: Shows a gray transparent overlay showing the current scroll offsets for the selected layer.

## Tile Viewer ##

<div class="imgBox"><div>
	<img src="/images/TileViewer.png" />
	<span>Tile Viewer</span>
</div></div>

The tile viewer can display the content of any type of memory (including video ram itself) as if it were tile data, in one of the various tile formats supported (2 BPP, 4 BPP, 8 BPP, etc.).  
For the SNES, there are a number of preset buttons that can be used to quickly view the tiles used by the matching background layer or sprites.  

Options:  

* **Source**: Selects which memory type to display in the viewer.  
* **Format**: Selects the tile format to use to interpret the data in memory.  
* **Address**: The offset in the selected memory type of the first byte of the first tile to display in the viewer.  
* **Size**: The number of bytes to display in the viewer (limited to 64kb)  
* **Columns**: The number of columns to use when displaying the data.  

There are also a number of display options:

* **Show tile grid**: Displays a 8x8 grid (or 16x16 depending on PPU settings) over the image to help distinguish each tile.

## Sprite Viewer ##

<div class="imgBox"><div>
	<img src="/images/SpriteViewer.png" />
	<span>Sprite Viewer</span>
</div></div>

The sprite viewer displays the contents of OAM RAM.
The left portion of the screen displays the sprites as they are on the screen, while the right half is a list of all sprites in OAM.
Sprites displayed in gray in the list are off-screen.

**Click** on a sprite in either side of the viewer to highlight the same sprite on the opposite half.

**Hide off-screen sprites**: When enabled, sprites that are off-screen will not be shown in the list.

## Palette Viewer ##

<div class="imgBox"><div>
	<img src="/images/PaletteViewer.png" />
	<span>Palette Viewer</span>
</div></div>

The Palette Viewer displays basic information about the current state of CG RAM (palette RAM).  
**Click** on a color in the viewer to see details about it.  

For **Game Boy** games, the selected palettes for the background and the two sprite palettes.  
For **Game Boy Color** games, the 8 background and 8 sprite palettes are shown.