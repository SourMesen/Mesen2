---
title: Assembler
weight: 11
chapter: false
---

<div class="imgBox"><div>
	<img src="/images/Assembler.png" />
	<span>Assembler</span>
</div></div>

The assembler allows writing new code, editing existing code and running arbitrary code. At the moment, the assembler can be used for the SNES's main CPU, the SA-1 and the Game Boy's CPU.

### Usage ###

Code is assembled on-the-fly, with the resulting byte code being shown on the right.  

Any compilation error will be shown in the list at the bottom -- **<kbd>double-click</kbd>** an error in the list to navigate to the line that caused it.

Once you are done editing the code, click `Apply` to write the code to the specified memory address - this can be used to create new code in RAM, for example, or alter existing code in the ROM.

{{% notice tip %}}
Any changes done to the ROM will remain in effect until the ROM is reloaded. If you want to save your modifications to a .sfc file, or as an IPS patch, you can use the **<kbd>File&rarr;Save ROM as</kbd>** or **<kbd>File&rarr;Save edits as IPS</kbd>** commands in the [debugger window](/debugging/debugger.html).
{{% /notice %}}

**Note**: When [editing an existing block of code](/debugging/debugger.html#how-to-edit-code), the assembler keeps track of how many bytes of code the original code contained. If the new code is too large to fit into the original block of code, a warning will be shown before applying the changes.


### Supported features ###

* All opcodes and addressing modes are supported.
* Hexadecimal ($ prefix), binary (% prefix) and decimal values can be used
* Labels can be used and defined in the code. Labels defined in the assembler are not permanent - they are discarded once the assembler is closed.
* The `.db` directive can be used to add arbitrary data to the output.


### Display Options ###

Syntax highlighting can be configured (or disabled) via the `View` menu.  
It is also possible to change the font size.