---
title: Enums
weight: 55
pre: ""
chapter: false
---

## eventType ##

**Syntax** 

	emu.eventType.[value]

**Values**

```text
nmi,            Triggered when an nmi occurs
irq,            Triggered when an irq occurs
startFrame,     Triggered at the start of a frame (cycle 0, scanline 0) (SNES only)
endFrame,       Triggered at the end of a frame (cycle 0 on the NMI scanline) (SNES only)
reset,          Triggered when a soft reset occurs
scriptEnded,    Triggered when the current Lua script ends (script window closed, execution stopped, etc.)
inputPolled,    Triggered when the emulation core polls the state of the input devices for the next frame
stateLoaded,    Triggered when a user manually loads a savestate
stateSaved,     Triggered when a user manually saves a savestate
gbStartFrame,   Triggered at the start of a frame (cycle 0, scanline 0) (Game Boy only)
gbEndFrame,     Triggered at the end of a frame (cycle 0, scanline 144) (Game Boy only)
```

**Description**  
Used by [addEventCallback](/apireference/callbacks.html#addeventcallback) / [removeEventCallback](/apireference/callbacks.html#removeeventcallback) calls.
 
## memCallbackType ##

**Syntax** 

	emu.memCallbackType.[value]

**Values**

```text
read,     Triggered when a read instruction is executed
write,    Triggered when a write instruction is executed
exec,     Triggered when any memory read occurs due to the CPU's code execution
```

**Description**  
Used by [addMemoryCallback](/apireference/callbacks.html#addmemorycallback) / [removeMemoryCallback](/apireference/callbacks.html#removememorycallback) calls.
 
## memType ##

**Syntax** 

	emu.memType.[value]

**Values**

```text
cpu,              CPU memory - Max: $FFFFFF          Warning: Reading or writing to this memory type may cause side-effects!
spc,              SPC memory - Max: $FFFF            Warning: Reading or writing to this memory type may cause side-effects!
sa1,              SPC memory - Max: $FFFFFF          Warning: Reading or writing to this memory type may cause side-effects!
gsu,              SPC memory - Max: $FFFFFF          Warning: Reading or writing to this memory type may cause side-effects!
cx4,              SPC memory - Max: $FFFFFF          Warning: Reading or writing to this memory type may cause side-effects!
gameboy,          Game Boy memory - Max: $FFFF       Warning: Reading or writing to this memory type may cause side-effects!
cgram,            CG/Palette RAM - Max: $1FF
oam,              OAM / Sprite RAM - Max: $21F
vram,             Video RAM - Max: $FFFF
prgRom,           PRG ROM - Size varies by game
workRam,          Work RAM - Max: $1FFFF
saveRam,          Save RAM - Size varies by game
gbPrgRom,         Game Boy PRG ROM - Size varies by game
gbWorkRam,        Game Boy Work RAM - Max: $1FFF (GB) or $7FFF (GBC)
gbCartRam,        Gmae Boy Cart/Save RAM - Size varies by game
gbVideoRam,       Game Boy Video RAM - Max: $1FFF (GB) or $3FFF (GBC)
cpuDebug,         CPU memory - Max: $FFFFFF          Same as non-debug variant, but avoid side-effects.
spcDebug,         SPC memory - Max: $FFFF            Same as non-debug variant, but avoid side-effects.
sa1Debug,         SA-1 memory - Max: $FFFFFF         Same as non-debug variant, but avoid side-effects.
gsuDebug,         GSU memory - Max: $FFFFFF          Same as non-debug variant, but avoid side-effects.
cx4Debug,         CX4 memory - Max: $FFFFFF          Same as non-debug variant, but avoid side-effects.
gameboyDebug,     Game Boy memory - Max: $FFFF       Same as non-debug variant, but avoid side-effects.
```	

**Description**  
Used by [read](/apireference/memoryaccess.html#read-readword) / [write](/apireference/memoryaccess.html#write-writeword) calls.


## counterOpType ##

**Syntax** 

	emu.counterOpType.[value]

**Values**

```text
read,
write,
exec
```	

**Description**  
Used by [getAccessCounters](/apireference/misc.html#getaccesscounters) calls.


## stepType ##

**Syntax** 

	emu.stepType.[value]

**Values**

```text
cpuInstructions,
ppuCycles
```	

**Description**  
Used by [execute](/apireference/emulation.html#execute) calls.


## cpuType ##

**Syntax** 

	emu.cpuType.[value]

**Values**

```text
cpu,       This represents the main SNES CPU (S-CPU)
spc,
dsp,
sa1,
gsu,
cx4,
gameboy
```	

**Description**  
Used by [registerMemoryCallback](/apireference/callbacks.html#registermemorycallback) calls.