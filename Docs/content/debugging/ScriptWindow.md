---
title: Script Window
weight: 25
chapter: false
---

<div class="imgBox"><div>
	<img src="/images/ScriptWindow.png" />
	<span>Script Window</span>
</div></div>

## Script Window ##

The Script Window allows [Lua](https://www.lua.org/) scripting via an [API](/apireference.html). Using this API, you can interact with the emulation core to perform a variety of things (e.g: logging, drawing, implementing an AI).

The code editor contains an autocomplete feature for all of Mesen-S' API -- typing `emu.` will display an autocomplete popup displaying all available functions.  Select a function in the list to see its parameters, return value and description.  
Multiple script windows can be opened at once to combine the effects of several scripts together.  

Scripts can be loaded from the disk and edited in any other text editor.  When using an external editor to modify the script, it is suggested to turn on the **<kbd>Script&rarr;Auto-reload when file changes</kbd>** option to automatically reload the script each time it is saved to the disk.

To start a script, press **<kbd>F5</kbd>**.  
To stop a script, press **<kbd>Escape</kbd>** or close the script window.

The Log Window at the bottom will display any Lua errors that occur while running the script.
Additionally, any calls to `emu.log()` will also display their content in the log window.

### Built-in Scripts ###

A number of built-in scripts are accessible from the script window's UI. They can be accessed from the **<kbd>File&rarr;Built-in Scripts</kbd>** menu, or directly from the toolbar.
These scripts serve as both examples for the Lua API and can also be useful in a number of ways.

Most of these scripts have been contributed by users (see the top of each script for proper credits) - **if you have useful/interesting scripts that you have written and would like to have included, let me know!**