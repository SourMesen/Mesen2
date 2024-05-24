# Mesen

Mesen is a multi-system emulator (NES, SNES, Game Boy, Game Boy Advance, PC Engine, Master System/Game Gear) for Windows, Linux and macOS built in C++ and C#.  

## Development Builds

[![Mesen](https://github.com/SourMesen/Mesen2/actions/workflows/build.yml/badge.svg)](https://github.com/SourMesen/Mesen2/actions/workflows/build.yml)

Latest development builds:  

* [Windows 10 / 11 (.NET 8)](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Windows%20-%20net8.0%29.zip)  
* [Windows 7 / 8 (.NET 6)](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Windows%20-%20net6.0%29.zip)  
* [macOS - Intel](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28macOS%20-%20macos-12%29.zip)  
* [macOS - Apple Silicon](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28macOS%20-%20macos-14%29.zip)  
* [Linux](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Linux%20-%20ubuntu-20.04%20-%20clang%29.zip)  
* [Linux - AppImage](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20(Linux%20x64%20-%20AppImage).zip)

#### Notes / limitations

**macOS**: The macOS build still has a number of limitations (e.g no gamepad support).

**SteamOS**: See [SteamOS.md](SteamOS.md)

## Requirements

To run Mesen, the following prerequisites must be installed:  

**Windows 10 / 11**: [.NET 8 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/8.0)  
**Windows 7 / 8**: [.NET 6 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/6.0) (.NET 8 is not supported on Windows 7 / 8)  
**Linux**: [.NET 8 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/8.0), SDL2  
**macOS**: [.NET 8 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/8.0), SDL2  

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
