# Mesen

Mesen is a multi-system emulator (NES, SNES, Game Boy and PC Engine) for Windows, Linux and macOS built in C++ and C#.  

## Releases

The latest release is available from the [releases](https://github.com/SourMesen/Mesen2/releases) page.

## Development Builds

[![Mesen](https://github.com/SourMesen/Mesen2/actions/workflows/build.yml/badge.svg)](https://github.com/SourMesen/Mesen2/actions/workflows/build.yml)

Latest development builds:  
[Windows](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Windows%29.zip)  
[Linux](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20%28Linux%20-%20ubuntu-20.04%20-%20clang%29.zip), AppImage (TO-DO)  

**macOS**: Dev builds aren't available for ARM Macs. It's recommended to build it yourself by running `make`. The macOS build is still **experimental** and a number of issues/bugs/limitations remain, but it seems to be usable for the most part.

## Requirements

To run Mesen, the following prerequisites must be installed:  

**Windows**: [.NET 6 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/6.0)  
**Linux**: [.NET 6 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/6.0), SDL2
	
 - When using an AppImage build, these prerequisites would not be necessary to be installed on the Linux machine.

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
