# Mesen

FAKE CHANGE - I'm really just trying to report an issue.

Mesen is a multi-system emulator (NES, SNES, Game Boy, PC Engine, Master System/Game Gear) for Windows, Linux and macOS built in C++ and C#.  

## Development Builds

[![Mesen](https://github.com/SourMesen/Mesen2/actions/workflows/build.yml/badge.svg)](https://github.com/SourMesen/Mesen2/actions/workflows/build.yml)

Latest development builds:  

* [Windows](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Windows%29.zip)  
* [macOS - Intel](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28macOS%20-%20macos-12%29.zip)  
* [macOS - Apple Silicon](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28macOS%20-%20macos-14%29.zip)  
* [Linux](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Linux%20-%20ubuntu-20.04%20-%20clang%29.zip)  
* [Linux - AppImage](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20(Linux%20x64%20-%20AppImage).zip)

**macOS**: The macOS build still has a number of limitations (e.g no gamepad support).

**SteamOS**: Running Mesen through the Steam Deck's _Game Mode_ is possible with some caveats regarding rendering the UI.
 	<details>
	<summary>SteamOS instructions and caveats</summary>
	<br>
	Due to Gamescope (SteamOS' compositor) not handling Avalonia UI's popups very well (a [solution](https://github.com/AvaloniaUI/Avalonia/pull/14366) is available but [has been reverted due to other issues](https://github.com/AvaloniaUI/Avalonia/pull/14573)), Mesen's menus for settings are not working through Gamescope unless running Mesen [through running KDE Plasma's Desktop through a script](https://www.reddit.com/r/SteamDeck/comments/zqgx9g/desktop_mode_within_gaming_mode_updated_for_new/).

 Installation instructions:
 * Download the **[Linux - AppImage](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20(Linux%20x64%20-%20AppImage).zip)** nightly build.
 * **Mark the AppImage as executable.** (right click > Properties > Permissions > Is executable)
 * Add the application as a non-Steam shortcut to be able to run it through Steam on both _Desktop Mode_ and _Game Mode_. (right click > Add to Steam)
 * Customise the non-Steam shortcut through Steam to your desire. (in Steam: search the AppImage's filename, right click > Properties; from there you can change the icon, shortcut name and launch options)
 * Create a folder called `Mesen.AppImage.Config` in the same directory where the AppImage is stored.
 * Run it the first time. When asking where to store the settings, choose the `Store the data in my user profile` option. ![254295455-88c1942d-b81f-48ee-a3a3-74b9f3ecd9b7-1](https://github.com/kevin-wijnen/Mesen2/assets/58944808/9f4ff1e3-4df6-4441-958b-ce96599ef69d)
 * Set up the controls as asked by Mesen.

**Due to Gamescope not rendering the UI menus, it is recommended to bind some keyboard shortcuts to L4/R4/L5/R5 (the Back Grip Buttons).** You can rebind controls in _Game Mode_ by clicking the Controller icon. You can save the layout by clicking the Cog icon (next to `Edit Layout`) > Export Layout > select `New Template` as the Export Type to use it across multiple shortcuts. 
It is recommended to:
* Bind `Control Key + O Key` to open the file picker for opening a game file.
* Bind `Escape Key` to pause emulation.
* Bind `F11` to enter in or out of fullscreen. 

**If sound does not work**, check if an audio device is chosen by Mesen. (in Mesen: Settings > Audio > General (Device))

**To make game-specific shortcuts**: Repeat the non-Steam shortcut step on the Mesen AppImage. Customise the new shortcut with a Launch Option (in Steam: right click > Properties; Launch Options). To find possible Launch Options, check the Command-line options menu (in Mesen: Help > Command-line options). When you want to supply a game with the shortcut, put the entire file location of the game in double quotes ("game-filepath") as the first part of the launch options. Add additional options (`--fullscreen` for example) _after_ the file location.
</details>

## Requirements

To run Mesen, the following prerequisites must be installed:  

**Windows**: [.NET 6 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/6.0)  
**Linux**: [.NET 6 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/6.0), SDL2  
**macOS**: [.NET 6 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/6.0), SDL2  

## Compiling

See [COMPILING.md](COMPILING.md)

## License

Mesen is available under the GPL V3 license.  Full text here: <http://www.gnu.org/licenses/gpl-3.0.en.html>

Copyright (C) 2023 Sour

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
