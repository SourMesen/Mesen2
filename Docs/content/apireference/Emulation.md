---
title: Emulation
weight: 15
pre: ""
chapter: false
---

## getState ##

**Syntax**  

    emu.getState()

**Return value**  
*Table* Current emulation state.

**Description**  
Return a table containing information about the state of the CPU, PPU and SPC.

## breakExecution ##

**Syntax**  

    emu.breakExecution()

**Return value**  
*None*

**Description**  
Breaks the execution of the game and displays the debugger window. 

## execute ##

**Syntax**  

    emu.execute(count, type)

**Parameters**  
count - *Integer* The number of cycles or instructions to run before breaking  
type - *Enum* See [executeCountType](/apireference/enums.html#executecounttype)

**Return value**  
*None*

**Description**  
Runs the emulator for the specified number of cycles/instructions and then breaks the execution.   

## reset ##

**Syntax**  

    emu.reset()

**Return value**  
*None*

**Description**  
Resets the current game.  

## resume ##

**Syntax**  

    emu.resume()

**Return value**  
*None*

**Description**  
Resumes execution after breaking. 


## rewind ##

**Syntax**  

    emu.rewind(seconds)

**Parameters**  
seconds - *Integer* The number of seconds to rewind

**Return value**  
*None*

**Description**  
Instantly rewinds the emulation by the number of seconds specified.  
**Note:** this can only be called from within a "StartFrame" event callback.  
