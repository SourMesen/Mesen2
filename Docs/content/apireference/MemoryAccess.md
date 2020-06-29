---
title: Memory Access
weight: 30
pre: ""
chapter: false
---

## read / readWord ##

**Syntax**  

    emu.read(address, type, signed = false)
	emu.readWord(address, type, signed = false)

**Parameters**  
address - *Integer* The address/offset to read from.  
type - *Enum* The type of memory to read from. See [memType](/apireference/enums.html#memtype).  
signed - (optional) *Boolean* If true, the value returned will be interpreted as a signed value.

**Return value**  
An 8-bit (read) or 16-bit (readWord) value.

**Description**  
Reads a value from the specified [memory type](/apireference/enums.html#memtype).  

When calling read / readWord, it is possible to trigger side-effects (such as register reads) when not using the "debug" memory types.
To avoid triggering side-effects, use the "debug" memory types.

## write / writeWord ##

**Syntax**  

    emu.write(address, value, type)
	emu.writeWord(address, value, type)

**Parameters**  
address - *Integer* The address/offset to write to.    
value - *Integer* The value to write.  
type - *Enum* The type of memory to write to. See [memType](/apireference/enums.html#memtype).  

**Return value**  
*None*

**Description**  
Writes an 8-bit or 16-bit value to the specified [memory type](/apireference/enums.html#memtype).  

Read-only memory such as PRG ROM can be written to using write/writeWord. Changes will remain in effect until the ROM is reloaded.  

When calling write / writeWord, it is possible to trigger side-effects (such as register writes) when not using the "debug" memory types.
To avoid triggering side-effects, use the "debug" memory types.


## getPrgRomOffset ##

**Syntax**  

    emu.getPrgRomOffset(address)

**Parameters**  
address - *Integer* A CPU address (Valid range: $0000-$FFFFFF)

**Return value**  
*Integer* The corresponding byte offset in PRG ROM

**Description**  
Returns an integer representing the byte offset of the specified CPU address in PRG ROM based on the mapper's current configuration. Returns -1 when the specified address is not mapped to PRG ROM.

## getLabelAddress ##

**Syntax**  

    emu.getLabelAddress(label)

**Parameters**  
label - *String* The label to look up

**Return value**  
*Integer* The corresponding CPU address

**Description**  
Returns the address of the specified label. This address can be used with the memory read/write functions (`read()`, `readWord()`, `write()`, `writeWord()`) using the `emu.memType.cpu` or `emu.memType.cpuDebug` memory types.
