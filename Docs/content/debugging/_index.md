---
title: Debugging Tools
weight: 5
chapter: false
toc: false
---

The debugging capabilities of Mesen are split across a number of different tools, including the debugger itself:

[Assembler](/debugging/assembler.html): Allows you to edit a game's code or run custom code on-the-fly.  
[Debugger](/debugging/debugger.html): View the code, add breakpoints, labels, watch values, and much more. A separate debugger window exists for each supported CPU.  
[Debug Log](/debugging/debuglog.html): Displays a log of various emulation-related events (uninitialized memory reads, SGB packets, etc.)  
[Event Viewer](/debugging/eventviewer.html): Visualize the timing of a variety of events (register read/writes, nmi, irq, etc.).  
[Memory Tools](/debugging/memorytools.html): Contains a hex editor and access counters for all memory types.      
[Performance Profiler](/debugging/performanceprofiler.html): Profiles the CPU's execution to help find bottlenecks in code.      
[PPU Viewers](/debugging/ppuviewers.html): Tools to display tiles, tilemaps, sprites and palettes.  
[Register Viewer](/debugging/registerviewer.html): Displays register/state information for most chips in the SNES/cartridge.  
[Script Window](/debugging/scriptwindow.html):  Allows the execution of Lua scripts, which can communicate with the emulation via an API.   
[Trace Logger](/debugging/tracelogger.html):  View or log to a file the execution trace of any of the supported CPUs.

Additionally, some other smaller features are available from the main debugger window.  e.g:

-  [Import labels from CA65/CC65/bass/RGBDS](/debugging/debuggerintegration.html)  
-  Save any modification done to ROM via the [Assembler](/debugging/assembler.html) or the [Memory Viewer](/debugging/memorytools.html#memory-viewer) as a new `.sfc` file, or as an `.ips` patch file  