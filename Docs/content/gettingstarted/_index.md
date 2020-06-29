---
title: Getting Started
weight: 1
chapter: false
---

## System Requirements ##

### Windows ###

* Windows 7, 8, 8.1 or 10
* DirectX 11
* .NET Framework 4.5+

### Linux ###

* glibc 2.24+
* Mono 5.18+
* SDL 2

## Installation ##

<div class="imgBox right"><div>
	<img src="/images/ConfigWizard.png" />
	<span>Configuration Wizard</span>
</div></div>

There is no installer for Mesen-S -- run the Mesen-S.exe application and a first-run configuration wizard will be shown.

**Data Storage Location**: This section of the wizard allows you to select where you prefer to keep the emulator's files.

**Input Mappings**: Select which input types you want to use to play games. There are built-in presets for:

* Xbox controllers
* PS4 controllers
* WASD keyboard layout 
* Arrow keyboard layout

You can select multiple presets at once, but only a single keyboard layout.

**Create a shortcut on my desktop**: Check this option if you want to add a Mesen-S shortcut to your desktop.

## Using Mesen-S ##

<div class="imgBox right"><div>
	<img src="/images/GameMenu.png" />
	<span>Game Menu</span>
</div></div>

The default configuration should work out of the box and allow you to get right into playing games.  

To load a game, use the **<kbd>File&rarr;Open</kbd>** command and select any supported file (or drag and drop a file):

 * `.sfc`: SNES games
 * `.gb` / `.gbc`: Game Boy and Game Boy Color games
 * `.bs`: BS-X / Satellaview games
 * `.spc`: SPC sound tracks

Additionally, `.zip` and `.7z` archives containing ROMs can be loaded directly.  
Once a game is loaded, you can pause, reset or stop the game via the `Game` menu.  

### Firmware / BIOS files ###

Some games require a firmware file (sometimes called a BIOS). Whenever a game that requires a firmware file is loaded, the emulator will automatically prompt you for the file. All firmware files are saved in the `Firmware` folder.  
The following firmware files are used:

* **BS-X/Satellaview**: For BS-X/Satellaview games, the BS-X cartridge's firmware is required. It is stored under the name `BS-X.bin`.
* **DSP**: This family of enhancement chips is used in a fairly large number of games, and several variations of the chips exist, each requiring their own firmware files. The split file format used by bsnes/higan is also supported, e.g: `dsp1.data.rom` and `dsp1.program.rom`.
	* `dsp1.rom`: DSP-1
	* `dsp1b.rom`: DSP-1b
	* `dsp2.rom`: DSP-2
	* `dsp3.rom`: DSP-3
	* `dsp4.rom`: DSP-4
	* `st010.rom`: ST010
	* `st011.rom`: ST011
* **Game Boy**: For Game Boy and Game Boy Color games, the boot roms are optional. When the files are not found, the open source GB/GBC boot roms written by LIJI32 (SameBoy author) will be used instead.
	* `dmg_boot.bin` (256 bytes): Game Boy Boot ROM
	* `cgb_boot.bin` (2304 bytes): Game Boy Color Boot ROM
* **Super Game Boy**: SGB emulation requires the SGB's ROM. Several versions exist based on region, and a different SGB2 ROM is also required for the SGB2. 
	* `SGB1.sfc` (256 kb): Super Game Boy
	* `SGB2.sfc` (512 kb): Super Game Boy 2
	* `sgb_boot.bin` (256 bytes): Super Game Boy CPU Boot ROM (optional)
	* `sgb2_boot.bin` (256 bytes): Super Game Boy 2 CPU Boot ROM (optional)

<div class="clear"></div>

### Game Boy Emulation ###

Alongside Super Game Boy support, Mesen-S can also emulate regular Game Boy and Game Boy Color games, without running them through the Super Game Boy.
To control which Game Boy model is used when loading `.gb` and `.gbc` files, use the `Model` dropdown in the [Game Boy options](/configuration/gameboy.html).  

All [debugging tools](/debugging.html) also support Game Boy and Game Boy Color games.

### SPC Player ###

<div class="imgBox"><div>
	<img src="/images/SpcPlayer.png" />
	<span>SPC Player</span>
</div></div>

SPC files are used to store music from SNES games.  These can be loaded like any other type of ROM.

When loading SPC files, the UI will display information about the track, artist, etc. while the track is playing.

<div class="clear"></div>

### Game Selection Screen ###

<div class="imgBox"><div>
	<img src="/images/GameSelectionScreen.png" />
	<span>Game Selection Screen</span>
</div></div>

The game selection screen is shown when no game is currently loaded -- it will display the last games you've played, along with a screenshot of the game at the point where you left off playing. The number of games shown depends on the window's size.

You can use this screen via the key bindings for player 1 - e.g press the d-pad to change selection, and the `A` button to start the game. You can also navigate the screen with your mouse -- use the arrows on each side of the screen to change game, and click on the game's screenshot to start playing.

<div class="clear"></div>

### Shortcut Keys ###

Mesen-S has a number of shortcut keys that you may find useful. All [shortcut keys](/configuration/preferences.html#shortcut-keys) can be customized in the [preferences](/configuration/preferences.html).

* <kbd>**Ctrl-O**</kbd>: Open a file
* <kbd>**Ctrl-R**</kbd>: Reset the game
* <kbd>**Escape**</kbd>: Pause/resume the game
* <kbd>**Alt-1 to Alt-6**</kbd>: Change the video scale.
* <kbd>**F1 to F7**</kbd>: Load save state in the corresponding slot.
* <kbd>**Shift-F1 to Shift-F7**</kbd>: Save a save state in the corresponding slot.
* <kbd>**Ctrl-S**</kbd>: Manually save a save state to a file.
* <kbd>**Ctrl-L**</kbd>: Manually load a save state from a file.
* <kbd>**Tab**</kbd>: Hold the tab key to fast forward the emulation (defaults to 300% speed)
* <kbd>**Backspace**</kbd>: Hold the backspace key to rewind the emulation, frame-by-frame.

{{% notice tip %}}
If you load a state, reset or power cycle by mistake, you can use the rewind feature to undo the action.
{{% /notice %}}

<div class="clear"></div>
