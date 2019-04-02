# Mesen-S

Mesen-S is a cross-platform SNES emulator for Windows & Linux built in C++ and C#.  
If you want to support this project, please consider making a donation:

[![Donate](https://www.mesen.ca/images/donate.png)](https://www.mesen.ca/Donate.php)

## Development Builds

Development builds of the latest commit are available from Appveyor. For release builds, see the **Releases** tab on GitHub.

**Warning:** These are development builds and may be ***unstable***. Using them may also increase the chances of your settings being corrupted, or having issues when upgrading to the next official release. Additionally, these builds are currently not optimized via PGO and will typically run 20-30% slower than the official release builds.

Windows: [![Build status](https://ci.appveyor.com/api/projects/status/cjk97u1yvwnae83x/branch/master?svg=true)](https://ci.appveyor.com/project/Sour/mesen-s/build/artifacts)

Linux: [![Build status](https://ci.appveyor.com/api/projects/status/arkaatgy94f23ll3/branch/master?svg=true)](https://ci.appveyor.com/project/Sour/mesen-s-hayo4/build/artifacts)

## Roadmap

Mesen-S is very early in its development and some features are still missing.  
The following should be added over time (in no particular order):

* Movies
* Netplay  
* Cheats  
* Additions/improvements in the debugging tools
* Lua scripting
* Support for the enhancement chips used in some games
* Libretro core (once the emulation core is stable/accurate enough)

## Compiling

See [COMPILING.md](COMPILING.md)

## License

Mesen is available under the GPL V3 license.  Full text here: <http://www.gnu.org/licenses/gpl-3.0.en.html>

Copyright (C) 2019 M. Bibaud

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
