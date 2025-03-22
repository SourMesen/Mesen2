# Mesen

Mesen is a multi-system emulator (NES, SNES, Game Boy, Game Boy Advance, PC Engine, SMS/Game Gear, WonderSwan) for Windows, Linux and macOS.  

## Development Builds

[![Mesen](https://github.com/SourMesen/Mesen2/actions/workflows/build.yml/badge.svg)](https://github.com/SourMesen/Mesen2/actions/workflows/build.yml)

#### <ins>Native builds</ins> ####

These builds don't require .NET to be installed and offer improved start up times.  

* [Windows 10 / 11](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Windows%20-%20net8.0%20-%20AoT%29.zip)  
* [Linux](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Linux%20-%20ubuntu-22.04%20-%20clang_aot%29.zip)  (requires **SDL2**)
* [macOS - Intel](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28macOS%20-%20macos-13%20-%20clang_aot%29.zip)  (requires **SDL2**)
* [macOS - Apple Silicon](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28macOS%20-%20macos-14%20-%20clang_aot%29.zip)  (requires **SDL2**)

#### <ins>.NET builds</ins> ####

These builds require **.NET 8** to be installed (except the Windows 7 build which requires .NET 6).  
For Linux and macOS, **SDL2** must also be installed.

* [Windows 10 / 11](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Windows%20-%20net8.0%29.zip)  
* [Windows 7 / 8 (.NET 6)](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Windows%20-%20net6.0%29.zip)  
* [macOS - Intel](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28macOS%20-%20macos-13%20-%20clang%29.zip)  
* [macOS - Apple Silicon](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28macOS%20-%20macos-14%20-%20clang%29.zip)  
* [Linux](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Linux%20-%20ubuntu-22.04%20-%20clang%29.zip)  
* [Linux - AppImage](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20(Linux%20x64%20-%20AppImage).zip)

#### <ins>Notes / limitations</ins> ####

**macOS**: The macOS build still has a number of limitations (e.g no gamepad support).

**SteamOS**: See [SteamOS.md](SteamOS.md)

## Compiling

See [COMPILING.md](COMPILING.md)

## License

Mesen is available under the GPL V3 license.  Full text here: <http://www.gnu.org/licenses/gpl-3.0.en.html>

Copyright (C) 2014-2024 Sour

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
