### Windows

1) Open the solution in Visual Studio 2022
2) Compile as Release/x64
3) Set the startup project to the "UI" project and run

### Linux

To compile Mesen under Linux you will need a version of clang or gcc which supports C++17.  

Additionally, the [.NET 6 SDK](https://learn.microsoft.com/en-us/dotnet/core/install/linux) must also be installed.

Examples:  
`LTO=true make` will compile with clang and LTO.  
`USE_GCC=true LTO=true make` will compile with gcc and LTO.  
