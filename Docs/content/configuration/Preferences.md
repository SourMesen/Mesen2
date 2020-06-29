---
title: Preferences
weight: 10
chapter: false
---

## General Options ##

<div class="imgBox"><div>
	<img src="/images/Preferences_General.png" />
	<span>General Options</span>
</div></div>

**Automatically check for updates**: When enabled, Mesen-S will check for a new version of the emulator every time the emulator is started.

**Only allow one instance of Mesen-S at a time**: When enabled, only a single copy of Mesen-S can be opened at the same time.  This is useful when using file associations to load games by double-clicking on the rom files.

### Pause/Background settings ###

**Pause when in**: 

* **Background**: When enabled, the emulator will automatically pause when in the background.
* **Menu and config dialogs**: When enabled, the emulator will automatically pause when in menus or configuration dialogs.
* **Debugging tools**: When enabled, the emulator will automatically pause when in debugging tools.

**Allow input when in background**: When enabled, gamepad input can still be used even if the window is in the background. *This does not work for keyboard key bindings.*

**Pause when a movie finishes playing**: When enabled, the emulator will automatically pause when a movie ends its playback.

### Misc. Settings ###

**Automatically apply IPS/BPS patches**: When enabled, any IPS or BPS patch file found in the same folder as the ROM file will automatically be applied. (i.e: when loading `MyRom.sfc`, if a file called `MyRom.ips` exists in the same folder, it will be loaded automatically.)

**Automatically hide the menu bar**: When enabled, the menu bar will automatically be hidden when not in use, even in windowed mode. *The menu bar is always hidden automatically when playing in fullscreen mode, whether this option is enabled or not.*

## Shortcut Keys ##

<div class="imgBox"><div>
	<img src="/images/Preferences_ShortcutKeys.png" />
	<span>Shortcut Keys</span>
</div></div>

Shortcut keys are user-defined shortcuts for various features in Mesen-S.  Some of these features can only be accessed via shortcut keys (e.g: `Fast Forward` and `Rewind`).  
Most of these options are also available through the main window's menu -- the shortcuts configured in this section will appear next to the matching action in the main window's menu.  

**To change a shortcut**, click on the button for the shortcut you want to change and press the key(s) you want to set for this shortcut.  Once you release a key, all the keys you had pressed will be assigned to the shortcut.

**To clear a key binding**, right-click on the button.

Available shortcuts:

* **Fast Forward (Hold button)**: Hold down this key to fast forward the game.
* **Toggle Fast Forward**: Start/stop fast forwarding.
* **Rewind (Hold button)**: Hold down this key to rewind the game in real-time.
* **Toggle Rewind**: Start/stop rewinding.
* **Rewind 10 seconds**: Instantly rewinds 10 seconds of gameplay.
* **Rewind 1 minute**: Instantly rewinds 1 minute of gameplay.
* **Pause**: Pauses or unpauses the game.
* **Reset**: Resets the game (equivalent to pressing the reset button on the SNES.)
* **Power Cycle**: Power cycles the game (equivalent to turning the power off and then back on.)
* **Reload ROM**: Reloads the ROM from the disk and power cycles the console.
* **Power Off**: Powers off the game, returning to the game selection screen.
* **Exit**: Exits the emulator.
* **Start/Stop Recording Video**: Starts (or stops) recording video (`.avi` file)
* **Start/Stop Recording Audio**: Starts (or stops) recording audio (`.wav` file)
* **Start/Stop Recording Movie**: Starts (or stops) recording a movie (`.msm` file)
* **Take Screenshot**: Takes a screenshot.
* **Run Single Frame**: Press to run a single frame and pause.
* **Set Scale 1x to 6x**: Sets the scale to the matching value.
* **Toggle Fullscreen Mode**: Enters/exits fullscreen mode.
* **Toggle Debug Information**: Turns on/off the debug screen overlay.
* **Toggle FPS Counter**: Turns on/off the FPS counter.
* **Toggle Game Timer**: Turns on/off the game timer.
* **Toggle Frame Counter**: Turns on/off the frame counter.
* **Toggle OSD (On-Screen Display)**: Turns on/off the OSD.
* **Toggle Display on Top**: Turns on/off the display on top option.
* **Enable/Disable Cheat Codes**: Press to toggle cheats on or off.
* **Toggle BG Layer 0-3**: Turns on/off the display of the corresponding background layer.
* **Toggle Sprite Layer**: Turns on/off the display of all sprites.
* **Enable All Layers**: Turns on the display of all background layers and all sprites.
* **Enable/Disable Audio**: Press to toggle audio output on or off.
* **Toggle Maximum Speed**: Toggles emulation speed between 100% and maximum speed.
* **Increase Speed**: Increases the emulation speed.
* **Decrease Speed**: Decreases the emulation speed.
* **Open File**: Displays the open file dialog.
* **Load Random Game**: Loads a random game from your game folder.
* **Select Next Save Slot**: Move to the next save state slot.
* **Select Previous Save Slot**: Move to the previous save state slot.
* **Save State**: Save the game's state in the currently selected slot.
* **Load State**: Load the game's state from the currently selected slot.
* **Save State - Slot X**: Save the game's state to the matching slot.
* **Load State - Slot X**: Load the game's state from the matching slot.
* **Save State to File**: Save the game's state to a user-specified file.
* **Load State from File**: Load the game's state from a user-specified file.
* **Select Save Slot X**: Select the specified save state slot.
* **Open Save State Menu**: Opens the save state menu (Graphical interface to save a new save state)
* **Open Load State Menu**: Opens the load state menu (Graphical interface to load an existing save state)

## Folders and File Associations ##

<div class="imgBox"><div>
	<img src="/images/Preferences_FoldersFiles.png" />
	<span>Folders and File Associations</span>
</div></div>

**File Associations**: Use these options to associate Mesen-S with these file types.  This will allow you to double-click on these files in a file explorer to open them in Mesen-S.

**Data Storage Location**: Mesen-S can either store its data in your user profile or in the same folder as the executable file itself.
This is configured when you launch Mesen-S for the first time, but can also be changed here.
When changing from one option to the other, Mesen-S will automatically copy all files from one folder to the other, allowing you to keep your save data and all other important files automatically.

**Folder Overrides**: On top of configuring the main folder where Mesen-S keeps its data, you may also specify custom locations for certain folders used by Mesen-S to store user-created files such as recordings or save data.

## Advanced Options ##

<div class="imgBox"><div>
	<img src="/images/Preferences_Advanced.png" />
	<span>Advanced Options</span>
</div></div>

### Window Settings ###

**Always display on top of other windows**: When enabled, the Mesen-S window will always be displayed above all other windows.

### UI Settings ###

**Disable on-screen display (OSD)**: When enabled, all on-screen messages are disabled.

**Disable game selection screen**: When enabled, the game selection screen is hidden and cannot be used.

**Display additional information in title bar**: When enabled, additional information is shown in the title bar (such as filter, scale, etc.), next to the game's name.

**Show FPS**: Displays an FPS counter on the screen.  The first number is the number of frames emulated, the second number is the number of frames displayed on the screen.  These values are usually identical, except when vertical sync is enabled.

**Show frame counter**: When enabled, the number of frames emulated since the last reset will be shown on the screen.

**Show game timer**: When enabled, the amount of time elapsed since the last reset will be shown on the screen.

**Show debug information**: When enabled, a debug information overlay will be shown on the screen.

### Miscellaneous Settings ###

**Keep rewind data for the last [x] minutes**: The rewind feature in Mesen-S periodically takes save states and keeps them in memory to allow the emulator to rewind the game. These save states take a minimal amount of memory (roughly 5MB per minute). To limit the amount of memory that Mesen-S can use for rewind data, this configures the number of minutes that it is possible to rewind for.
