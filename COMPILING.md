## Windows

1) Open the solution in Visual Studio 2022
    * Mesen uses the "Desktop Development With C++" workload and the ".NET Desktop Development" workload
    * You do **not** need to install them in advance. If you don't have them, Visual Studio will install them when you open the project.
2) Compile as `Release`/`x64`
3) Set the startup project to the `UI` project and run

### Testing

There is a [pester](https://pester.dev/) powershell test script in `UI/tests` that will verify the prebuild steps for the `UI` project.


1. [Install pester](https://pester.dev/docs/introduction/installation) if needed.
1. Run from any powershell console:

```cmd
$ .\UI\tests\prebuild_windows.test.ps1
```


## Linux

To build under Linux you need a version of Clang or GCC that supports C++17.  

Additionally, SDL2 and the [.NET 8 SDK](https://learn.microsoft.com/en-us/dotnet/core/install/linux) must also be installed.

Once SDL2 and the .NET 8 SDK are installed, run `make` to compile with Clang.  
To compile with GCC instead, use `USE_GCC=true make`.  
**Note:** Mesen usually runs faster when built with Clang instead of GCC.


## macOS

To build macOS, install SDL2 (i.e via Homebrew) and the [.NET 8 SDK](https://dotnet.microsoft.com/en-us/download/dotnet/8.0).  

Once SDL2 and the .NET 8 SDK are installed, run `make`.