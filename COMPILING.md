### Windows

#### *Standalone*

1) Open the solution in Visual Studio 2019
2) Compile as Release/x64
3) Run

#### *Libretro*

1) Open the solution in Visual Studio 2019
2) Compile as Libretro/x64
3) Use the "mesen-s_libretro.dll" file in bin/(x64 or x86)/Libretro/mesen-s_libretro.dll

Note: It's also possible to build the Libretro core via MINGW by using the makefile in the Libretro subfolder.

### Linux

#### *Standalone*

To compile Mesen-S under Linux you will need a relatively recent version of clang or gcc that supports the C++17 filesystem API.) Additionally, Mesen-S has the following dependencies:

* Mono 5.18+  (package: mono-devel) 
* SDL2  (package: libsdl2-dev)

**Note:** **Mono 5.18 or higher is recommended**, some older versions of Mono (e.g 4.2.2) have some stability and performance issues which can cause crashes and slow down the UI.
The default Mono version in Ubuntu 18.04 is 4.6.2 (which also causes some layout issues in Mesen-S).  To install the latest version of Mono, follow the instructions here: https://www.mono-project.com/download/stable/#download-lin

The makefile contains some more information at the top.  Running "make" will build the x64 version by default, and then "make run" should start the emulator.
LTO is supported under clang, which gives a large performance boost (25-30%+), so turning it on is highly recommended (see makefile for details):

Examples:  
`LTO=true make` will compile with clang and LTO.  
`USE_GCC=true LTO=true make` will compile with gcc and LTO.  

#### *Libretro*

To compile the Libretro core you will need a version of clang/gcc that supports C++14.
Run "make" from the "Libretro" subfolder to build the Libretro core.
