---
title: Miscellaneous
weight: 40
pre: ""
chapter: false
---

## Access Counters ##

### getAccessCounters ###

**Syntax**  

    emu.getAccessCounters(memType, counterOpType)

**Parameters**  
memType - *Enum* A value from the [emu.memType](/apireference/enums.html#memtype) enum  
counterOpType - *Enum* A value from the [emu.counterOpType](/apireference/enums.html#counteroptype) enum  
	
**Return value**  
*Array* 32-bit integers

**Description**  
Returns an array of access counters for the specified memory and operation types.


### resetAccessCounters ###

**Syntax**  

    emu.resetAccessCounters()

**Return value**  
*None* 

**Description**  
Resets all access counters.


## Misc ##

### getLogWindowLog ###

**Syntax**  

    emu.getLogWindowLog()

**Return value**  
*String* A string containing the log shown in the log window

**Description**  
Returns the same text as what is shown in the emulator's Log Window.

### getRomInfo ###

**Syntax**  

    emu.getRomInfo()

**Return value**  
*Table* Information about the current ROM with the following structure:

```text
name: string,            Filename of the current ROM
path: string,            Full path to the current ROM (including parent compressed archive when needed)
fileSha1Hash: string,    The SHA1 hash for the whole ROM file
```
	
**Description**  
Returns information about the ROM file that is currently running.


### getScriptDataFolder ###

**Syntax**  

    emu.getScriptDataFolder()

**Return value**  
*String* The script's data folder

**Description**  
This function returns the path to a unique folder (based on the script's filename) where the script should store its data (if any data needs to be saved).  
The data will be saved in subfolders inside the LuaScriptData folder in Mesen-S' home folder.


### takeScreenshot ###

**Syntax**  

    emu.takeScreenshot()

**Return value**  
*String* A binary string containing a PNG image. 

**Description**  
Takes a screenshot and returns a PNG file as a string.  
The screenshot is not saved to the disk.

