---
title: Callbacks
weight: 5
pre: ""
chapter: false
---

## addEventCallback ##

**Syntax**
    
    emu.addEventCallback(function, type)

**Parameters**  
function - A Lua function.  
type - *Enum* See [eventType](/apireference/enums.html#eventtype).

**Return value**  
Returns an integer value that can be used to remove the callback by calling [removeEventCallback](#removeeventcallback). 

**Description**  
Registers a callback function to be called whenever the specified event occurs.  
The callback function receives no parameters.

## removeEventCallback ##

**Syntax**
    
    emu.removeEventCallback(reference, type)

**Parameters**  
reference - The value returned by the call to [addEventCallback](#addeventcallback).  
type - *Enum* See [eventType](/apireference/enums.html#eventtype).

**Return value**  
*None*

**Description**  
Removes a previously registered callback function.

## addMemoryCallback ##

**Syntax**
    
    emu.addMemoryCallback(function, type, startAddress [, endAddress, cpuType])

**Parameters**  
function - A Lua function.  
type - *Enum* See [memCallbackType](/apireference/enums.html#memcallbacktype)  
startAddress - *Integer* Start of the CPU memory address range to register the callback on.  
endAddress - (optional) *Integer* End of the CPU memory address range to register the callback on.  
cpuType - (optional) *Enum* See [cpuType](/apireference/enums.html#cputype)  

**Return value**  
Returns an integer value that can be used to remove the callback by calling [removeMemoryCallback](#removememorycallback). 

**Description**  
Registers a callback function to be called whenever the specified memory address is read/written/executed (based on `type`) by the specific CPU (`cpuType` - When no CPU type is specified, this defaults to emu.cpuType.cpu).  
The callback function receives 2 parameters `address` and `value` that correspond to the address being written to or read from, and the value that is being read/written.  

For reads, the callback is called *after* the read is performed.  
For writes, the callback is called *before* the write is performed.  


## removeMemoryCallback ##

**Syntax**
    
    emu.removeMemoryCallback(reference, type, startAddress [, endAddress, cpuType])

**Parameters**  
reference - The value returned by the call to [addMemoryCallback](#addmemorycallback).  
type - *Enum* See [memCallbackType](/apireference/enums.html#memcallbacktype).   
startAddress - *Integer* Start of the CPU memory address range to unregister the callback from.  
endAddress - (optional) *Integer* End of the CPU memory address range to unregister the callback from.  
cpuType - (optional) *Enum* See [cpuType](/apireference/enums.html#cputype)  

**Return value**  
*None*

**Description**  
Removes a previously registered callback function.
