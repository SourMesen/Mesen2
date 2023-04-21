-- 
-- Annotations of Mesen lua script for LSP(language server protocol).
-- See: https://github.com/LuaLS/lua-language-server/wiki/Annotations
-- You could export this definition and use it for code completion in other
-- code editor with Lua annotation support or Lua LSP.
--

--- @meta emu

--- Used by emu.addMemoryCallback() and emu.removeMemoryCallback()
--- @enum callbackType

--- Used by emu.addMemoryCallback() and emu.removeMemoryCallback()
--- @class enum.callbackType
--- Callback is called when data is read
--- @field read callbackType
--- Callback is called when data is written
--- @field write callbackType
--- Callback is called when the CPU starts executing an instruction
--- @field exec callbackType

--- Used by emu.addCheat()
--- @enum cheatType

--- Used by emu.addCheat()
--- @class enum.cheatType
--- Game Genie (NES)
--- @field nesGameGenie cheatType
--- Pro Action Rocky (NES)
--- @field nesProActionRocky cheatType
--- Custom Address:Value code (NES)
--- @field nesCustom cheatType
--- Game Genie (GB)
--- @field gbGameGenie cheatType
--- Game Shark (GB)
--- @field gbGameShark cheatType
--- Game Genie (SNES)
--- @field snesGameGenie cheatType
--- Pro Action Replay (SNES)
--- @field snesProActionReplay cheatType
--- 24-bit address format (PC Engine)
--- @field pceRaw cheatType
--- 21-bit address format (PC Engine)
--- @field pceAddress cheatType

--- Used by emu.getAccessCounters()
--- @enum counterType

--- Used by emu.getAccessCounters()
--- @class enum.counterType
--- Returns the number of times each byte was read
--- @field readCount counterType
--- Returns the number of times each byte was written
--- @field writeCount counterType
--- Returns the number of times each byte was executed
--- @field execCount counterType
--- Returns the time at which each byte was last read
--- @field lastReadClock counterType
--- Returns the time at which each byte was last written
--- @field lastWriteClock counterType
--- Returns the time at which each byte was last executed
--- @field lastExecClock counterType

--- Used by several APIs to specify which CPU the API call applies to.
--- @enum cpuType

--- Used by several APIs to specify which CPU the API call applies to.
--- @class enum.cpuType
--- SNES - Main CPU - S-CPU 5A22 (65186)
--- @field snes cpuType
--- SNES - SPC
--- @field spc cpuType
--- SNES - DSP-n
--- @field necDsp cpuType
--- SNES - SA-1
--- @field sa1 cpuType
--- SNES - CX4
--- @field cx4 cpuType
--- Game Boy - Main CPU (can also be used in SGB mode)
--- @field gameboy cpuType
--- NES - Main CPU - 2A03 (6502)
--- @field nes cpuType
--- PC Engine - Main CPU - HuC6280
--- @field pce cpuType

--- Used by emu.selectDrawSurface() and emu.getDrawSurfaceSize()
--- @enum drawSurface

--- Used by emu.selectDrawSurface() and emu.getDrawSurfaceSize()
--- @class enum.drawSurface
--- Console's framebuffer. Drawings appear in screenshots/videos.
--- @field consoleScreen drawSurface
--- Separate surface with a configurable resolution. Drawings not shown in screenshots/videos.
--- @field scriptHud drawSurface

--- Used by emu.addEventCallback() and emu.removeEventCallback()
--- @enum eventType

--- Used by emu.addEventCallback() and emu.removeEventCallback()
--- @class enum.eventType
--- Triggered when an NMI occurs (not available on some consoles)
--- @field nmi eventType
--- Triggered when an IRQ occurs
--- @field irq eventType
--- Triggered when a frame starts (typically once vertical blank ends)
--- @field startFrame eventType
--- Triggered when a frame ends (typically once vertical blank starts)
--- @field endFrame eventType
--- Triggered when the console is reset (not available on some consoles)
--- @field reset eventType
--- Triggered when the Lua script is stopped
--- @field scriptEnded eventType
--- Triggered after the emulator updates the state of all input devices (once per frame)
--- @field inputPolled eventType
--- Triggered when a savestate is manually loaded
--- @field stateLoaded eventType
--- Triggered when a savestate is manually saved
--- @field stateSaved eventType
--- Triggered when code execution breaks (e.g breakpoint, step, etc.)
--- @field codeBreak eventType

--- Used by several APIs to specify which memory type to access/use.
--- @enum memType

--- Used by several APIs to specify which memory type to access/use.
--- @class enum.memType
--- SNES - S-CPU memory
--- @field snesMemory memType
--- SNES - SPC memory
--- @field spcMemory memType
--- SNES - SA-1 memory
--- @field sa1Memory memType
--- SNES - DSP-n memory
--- @field necDspMemory memType
--- SNES - GSU memory
--- @field gsuMemory memType
--- SNES - CX4 memory
--- @field cx4Memory memType
--- Game Boy - CPU memory
--- @field gameboyMemory memType
--- NES - CPU memory
--- @field nesMemory memType
--- NES - PPU memory
--- @field nesPpuMemory memType
--- PC Engine - CPU memory
--- @field pceMemory memType
--- SNES - S-CPU memory (no read/write side-effects)
--- @field snesDebug memType
--- SNES - SPC memory (no read/write side-effects)
--- @field spcDebug memType
--- SNES - SA-1 memory (no read/write side-effects)
--- @field sa1Debug memType
--- SNES - DSP-n memory (no read/write side-effects)
--- @field necDspDebug memType
--- SNES - GSU memory (no read/write side-effects)
--- @field gsuDebug memType
--- SNES - CX4 memory (no read/write side-effects)
--- @field cx4Debug memType
--- Game Boy - CPU memory (no read/write side-effects)
--- @field gameboyDebug memType
--- NES - CPU memory (no read/write side-effects)
--- @field nesDebug memType
--- NES - PPU memory (no read/write side-effects)
--- @field nesPpuDebug memType
--- PC Engine - CPU memory (no read/write side-effects)
--- @field pceDebug memType
--- SNES - PRG ROM
--- @field snesPrgRom memType
--- SNES - Work RAM
--- @field snesWorkRam memType
--- SNES - Save RAM
--- @field snesSaveRam memType
--- SNES - Video RAM
--- @field snesVideoRam memType
--- SNES - Sprite RAM (OAM)
--- @field snesSpriteRam memType
--- SNES - Palette RAM (CGRAM)
--- @field snesCgRam memType
--- SNES - SPC - RAM
--- @field spcRam memType
--- SNES - SPC - IPL ROM
--- @field spcRom memType
--- SNES - DSP-n - Program ROM
--- @field dspProgramRom memType
--- SNES - DSP-n - Data ROM
--- @field dspDataRom memType
--- SNES - DSP-n - Data RAM
--- @field dspDataRam memType
--- SNES - SA-1 - IRAM
--- @field sa1InternalRam memType
--- SNES - GSU - Work RAM
--- @field gsuWorkRam memType
--- SNES - CX4 - Data RAM
--- @field cx4DataRam memType
--- SNES - BS-X - PSRAM
--- @field bsxPsRam memType
--- SNES - BS-X - Memory Pack
--- @field bsxMemoryPack memType
--- Game Boy - PRG ROM
--- @field gbPrgRom memType
--- Game Boy - Work RAM
--- @field gbWorkRam memType
--- Game Boy - Cart/Save RAM
--- @field gbCartRam memType
--- Game Boy - High RAM
--- @field gbHighRam memType
--- Game Boy - Boot ROM
--- @field gbBootRom memType
--- Game Boy - Video RAM
--- @field gbVideoRam memType
--- Game Boy - Sprite RAM
--- @field gbSpriteRam memType
--- NES - PRG ROM
--- @field nesPrgRom memType
--- NES - System RAM
--- @field nesInternalRam memType
--- NES - Work RAM
--- @field nesWorkRam memType
--- NES - Save RAM
--- @field nesSaveRam memType
--- NES - Nametable RAM (CIRAM)
--- @field nesNametableRam memType
--- NES - Sprite RAM (OAM)
--- @field nesSpriteRam memType
--- NES - Secondary Sprite RAM
--- @field nesSecondarySpriteRam memType
--- NES - Palette RAM
--- @field nesPaletteRam memType
--- NES - CHR RAM
--- @field nesChrRam memType
--- NES - CHR ROM
--- @field nesChrRom memType
--- PC Engine - HuCard ROM
--- @field pcePrgRom memType
--- PC Engine - Work RAM
--- @field pceWorkRam memType
--- PC Engine - Save RAM
--- @field pceSaveRam memType
--- PC Engine - CD-ROM Unit RAM
--- @field pceCdromRam memType
--- PC Engine - Card RAM
--- @field pceCardRam memType
--- PC Engine - ADPCM RAM
--- @field pceAdpcmRam memType
--- PC Engine - Arcade Card RAM
--- @field pceArcadeCardRam memType
--- PC Engine - Video RAM (VDC)
--- @field pceVideoRam memType
--- PC Engine - Video RAM (VDC2 - SuperGrafx only)
--- @field pceVideoRamVdc2 memType
--- PC Engine - Sprite RAM (VDC)
--- @field pceSpriteRam memType
--- PC Engine - Sprite RAM (VDC2 - SuperGrafx only)
--- @field pceSpriteRamVdc2 memType
--- PC Engine - Palette RAM (VCE)
--- @field pcePaletteRam memType

--- Used by emu.step()
--- @enum stepType

--- Used by emu.step()
--- @class enum.stepType
--- Steps the specified number of instructions
--- @field step stepType
--- Steps out of the current subroutine (not available for all CPUs)
--- @field stepOut stepType
--- Steps over the current subroutine call (not available for all CPUs)
--- @field stepOver stepType
--- Steps the specified number of CPU cycles (not available for all CPUs)
--- @field cpuCycleStep stepType
--- Steps the specified number of scanline cycles
--- @field ppuStep stepType
--- Steps the specified number of scanlines
--- @field ppuScanline stepType
--- Steps the specified number of video frames
--- @field ppuFrame stepType
--- Breaks on the specified scanline number
--- @field specificScanline stepType
--- Breaks on the next NMI event
--- @field runToNmi stepType
--- Breaks on the next IRQ event
--- @field runToIrq stepType

--- @class Emu
--- @field callbackType enum.callbackType
--- @field cheatType enum.cheatType
--- @field counterType enum.counterType
--- @field cpuType enum.cpuType
--- @field drawSurface enum.drawSurface
--- @field eventType enum.eventType
--- @field memType enum.memType
--- @field stepType enum.stepType
--- lua api entries
emu = {}

--- Adds the specified cheat code.
---
--- Note: Cheat codes added via this function are not permanent and not visible in the UI.
--- @param cheatType cheatType Cheat type/format
--- @param cheatCode string Cheat code
function emu.addCheat(cheatType, cheatCode)
end

--- Registers a callback function to be called whenever the specified event occurs.
--- The callback function receives 1 parameter ("cpuType") which is an enum value (emu.cpuType) which indicates which CPU triggered the event (useful to distinguish between identical events triggered by different CPUs, e.g Super Game Boy).
--- @param callback function Lua function to call when the event occurs
--- @param eventType eventType Event type
--- @return integer Value that can be used to remove the callback by calling emu.removeEventCallback().
function emu.addEventCallback(callback, eventType)
end

--- Registers a callback function to be called whenever the specified event occurs.
--- The callback function receives 2 parameters ("address" and "value") that correspond to the address being written to or read from, and the value that is being read/written.
---
--- For reads, the callback is called after the read is performed.
--- For writes, the callback is called before the write is performed.
---
--- If the callback returns an integer value, it will replace the value - you can alter the results of read/write operation by using this.
--- @param callback function Lua function to call when the event occurs
--- @param callbackType callbackType Callback type
--- @param startAddress integer Start of the address range
--- @param endAddress integer? End of the address range
--- @param cpuType cpuType? CPU used for the callback
--- @param memoryType memType? Memory type for the callback
--- @return integer Value that can be used to remove the callback by calling emu.removeMemoryCallback().
function emu.addMemoryCallback(callback, callbackType, startAddress, endAddress, cpuType, memoryType)
end

--- Breaks the execution.
function emu.breakExecution()
end

--- Removes all active cheat codes.
---
--- Note: This has no impact on cheat codes saved in the UI, but it will disable them temporarily.
function emu.clearCheats()
end

--- Removes all drawn shapes from the screen.
function emu.clearScreen()
end

--- Converts an address between CPU addressing mode and ROM/RAM addressing mode.
--- When a ROM/RAM address is given, a CPU address matching that value is returned, if the value is mapped.
--- When a CPU address is given, the corresponding ROM/RAM address is returned, if the address is mapped to ROM/RAM.
--- @param address integer Address to convert
--- @param memoryType memType? Memory type
--- @param cpuType cpuType? CPU used for the conversion
--- @return {address: integer, memType: memType}
function emu.convertAddress(address, memoryType, cpuType)
end

--- Creates a savestate and returns it as a binary string.
---
--- Note: This can only be called from inside an "exec" memory callback.
--- @return string Binary string containing the savestate.
function emu.createSavestate()
end

--- Displays a message on the main window in the format "[category] text"
--- @param category string The category is the portion shown between brackets
--- @param text string Text to show on the screen
function emu.displayMessage(category, text)
end

--- Draws a line between (x, y) to (x2, y2) using the specified color for a specific number of frames.
--- @param x integer X position (start of line)
--- @param y integer Y position (start of line)
--- @param x2 integer X position (end of line)
--- @param y2 integer Y position (end of line)
--- @param color integer? Color
--- @param duration integer? Number of frames to display
--- @param delay integer? Number of frames to wait before drawing the line
function emu.drawLine(x, y, x2, y2, color, duration, delay)
end

--- Draws a pixel at the specified (x, y) coordinates using the specified color for a specific number of frames.
--- @param x integer X position
--- @param y integer Y position
--- @param color integer Color
--- @param duration integer? Number of frames to display
--- @param delay integer? Number of frames to wait before drawing the pixel
function emu.drawPixel(x, y, color, duration, delay)
end

--- Draws a rectangle between (x, y) to (x+width, y+height) using the specified color for a specific number of frames. If fill is false, only the rectangle's outline will be drawn.
--- @param x integer X position
--- @param y integer Y position
--- @param width integer Width
--- @param height integer Height
--- @param color integer? Color
--- @param fill boolean? Whether or not to draw an outline, or a filled rectangle.
--- @param duration integer? Number of frames to display
--- @param delay integer? Number of frames to wait before drawing the rectangle
function emu.drawRectangle(x, y, width, height, color, fill, duration, delay)
end

--- Draws text at (x, y) using the specified text and colors for a specific number of frames.
--- @param x integer X position
--- @param y integer Y position
--- @param text string Text to display
--- @param textColor integer? Color to use for the text
--- @param backgroundColor integer? Color to use for the background
--- @param maxWidth integer? Max width (pixels) - wraps to next line when reached
--- @param duration integer? Number of frames to display
--- @param delay integer? Number of frames to wait before drawing the text
function emu.drawString(x, y, text, textColor, backgroundColor, maxWidth, duration, delay)
end

--- Returns an array of access counters for the specified memory and operation types.
--- @param counterType counterType Counter type
--- @param memoryType memType Memory type
--- @return integer[] Array of ints
function emu.getAccessCounters(counterType, memoryType)
end

--- Returns a table containing the full size, visible size and overscan size for the selected draw surface.
--- @param surface drawSurface? Draw surface
--- @return { width: integer, height: integer, visibleWidth: integer, visibleHeight: integer, overscan: {top: integer, bottom: integer, left: integer, right: integer}}
function emu.getDrawSurfaceSize(surface)
end

--- Returns a table containing the state of all buttons for the selected port/controller. The table's content varies based on the controller type.
--- @param port integer Port number
--- @param subPort integer? Subport number - this is used for multitap-like adapters.
--- @return { ["a"| "b" | "select" | "start" | "up" | "down" | "left" | "right" | string ]: boolean } Content varies based on controller type.
function emu.getInput(port, subPort)
end

--- Returns a table containing the address and memory type for the specified label.
--- @param label string Label
--- @return { address: integer, memType: memType} . Note: Returns nil when the specified label could not be found.
function emu.getLabelAddress(label)
end

--- Returns the same text as what is shown in the emulator's log window.
--- @param label string Label
--- @return table A string containing the log shown in the log window
function emu.getLogWindowLog(label)
end

--- Returns the size (in bytes) of the specified memory type.
--- @param memoryType memType Memory type
--- @return integer Size of the specified memory type
function emu.getMemorySize(memoryType)
end

--- Returns a table containing the position and the state of all 3 buttons.
--- @return { x : integer, y : integer, relativeX : integer, relativeY : integer, left : boolean, middle : boolean, right : boolean }
function emu.getMouseState()
end

--- Returns the color (in ARGB format) of the screen's output for the specified coordinates.
--- @param x integer X position
--- @param y integer Y position
--- @return integer ARGB color
function emu.getPixel(x, y)
end

--- Returns information about the ROM file that is currently running.
--- @return { name : string, path : string, fileSha1Hash : string }
function emu.getRomInfo()
end

--- Returns an array of ARGB values with the contents of the console's screen - can be used with emu.setScreenBuffer() to modify the screen's contents.
---
--- Note: The size of the array varies based on the console, game, and sometimes scene. Use emu.getScreenSize() to get the screen's current dimensions.
--- @return integer[] Array of ARGB values
function emu.getScreenBuffer()
end

--- Returns a table containing the size of the console's current screen output.
--- @return { width : integer, height : integer }
function emu.getScreenSize()
end

--- This function returns the path to a unique folder (based on the script's filename) where the script should store its data (if any data needs to be saved). The data will be saved in subfolders inside the LuaScriptData folder in Mesen's home folder.
---
--- Note: This function will return an empty string if the "Allow access to I/O and OS functions" option is disabled.
--- @return string The folder's path
function emu.getScriptDataFolder()
end

--- Returns a table containing key-value pairs that describe the console's current state.
---
--- Note: The name of the values returned may change from one version to another. Some values may represent the emulator's internal state and may not be useful (these will be hidden in future versions.)
--- @return table Content varies for each console and game.
function emu.getState()
end

--- Returns whether or not a specific key is pressed. The "keyName" must be the same as the string shown in the UI when the key is bound to a button.
--- @param keyName string Name of the key to check
--- @return boolean The key's state (true when pressed)
function emu.isKeyPressed(keyName)
end

--- Loads a savestate from a binary string.
---
--- Note: This can only be called from inside an "exec" memory callback.
--- @param state string Binary data containing the savestate
function emu.loadSavestate(state)
end

--- Logs the specified string in the script's log window - useful for debugging scripts.
--- @param text string Text to log
function emu.log(text)
end

--- Measures the specified string and returns a table containing the width and height that the string would take when drawn.
--- @param text string String to measure
--- @param maxWidth integer? Max width (pixels) - wraps to next line when reached
--- @return { width : integer, height : integer }
function emu.measureString(text, maxWidth)
end

--- Reads an 8-bit value from the specified address and memory type.
---
--- Note: When using "memType.[cpuName]" memory types, side-effects can occur from reading a value. Use the "memType.[cpuName]Debug" enum values to avoid side-effects.
--- @param address integer Address to read from
--- @param memoryType memType Memory type to read from
--- @param signed boolean When true, the return value is an 8-bit signed value.
--- @return integer An 8-bit (signed or unsigned) value.
function emu.read(address, memoryType, signed)
end

--- Reads a 16-bit value from the specified address and memory type.
---
--- Note: When using "memType.[cpuName]" memory types, side-effects can occur from reading a value. Use the "memType.[cpuName]Debug" enum values to avoid side-effects.
--- @param address integer Address to read from
--- @param memoryType memType Memory type to read from
--- @param signed boolean When true, the return value is a 16-bit signed value.
--- @return integer A 16-bit (signed or unsigned) value.
function emu.readWord(address, memoryType, signed)
end

--- Resets the current game. If the console does not have a reset button, this will have the same effect as power cycling.
function emu.reset()
end

--- Resets all access counters.
function emu.resetAccessCounters()
end

--- Resumes execution after a break.
function emu.resume()
end

--- Instantly rewinds the emulation by the number of seconds specified.
---
--- Note: This can only be called from inside an "exec" memory callback.
--- @param seconds integer Number of seconds to rewind
function emu.rewind(seconds)
end

--- Selects the surface on which any subsequent draw call will be drawn to.
---
--- consoleScreen: This surface is the same as the console's output and is a fixed resolution. Anything drawn here will appear in screenshots/videos.
---
--- scriptHud: This surface is independent to the console's output and has a configurable resolution. Drawings done on this surface will not appear in screenshots/videos.
---
--- Note: setScreenBuffer always draws to the "consoleScreen" surface.
--- @param surface drawSurface Draw surface
--- @param scale integer? Scale to use for the "scriptHud" surface (max: 4)
function emu.selectDrawSurface(surface, scale)
end

--- Sets the input state for the specified port. Buttons enabled or disabled via setInput will keep their state until the next inputPolled event.
---
--- Note: If a button's value is not specified in the "input" argument, then the player retains control of that button. For example, "emu.setInput({ select = false, start = false }, 0)" will prevent the player 1 from using both the start and select buttons, but all other buttons will still work as normal.
---
--- It is recommended to use this function within a callback for the inputPolled event. Otherwise, the inputs may not be applied before the ROM has the chance to read them.
--- @param input table Controller state to apply to the port, same format as the return value of getState().
--- @param port integer Port number
--- @param subPort integer? Subport number - this is used for multitap-like adapters.
function emu.setInput(input, port, subPort)
end

--- Replaces the current frame with the contents of the specified array.
--- @param screenBuffer integer[] Array of integers in ARGB format
function emu.setScreenBuffer(screenBuffer)
end

--- Changes the state of the emulator to match the values provided in the "state" parameter.
---
--- Note: The name of the values returned may change from one version to another. Some values may represent the emulator's internal state and should not be changed (access to these will be removed in future versions.)
--- @param state table A key-value table containing the state to be applied.
function emu.setState(state)
end

--- Breaks the emulation's execution when the step conditions are reached.
--- @param count integer Number of cycles/frames/etc.
--- @param stepType stepType Step type
--- @param cpuType cpuType? CPU type
function emu.step(count, stepType, cpuType)
end

--- Stops the emulation and returns the specified exit code (when used with the --testRunner command line option).
--- @param exitCode integer The exit code that the Mesen process will return.
function emu.stop(exitCode)
end

--- Takes a screenshot and returns a PNG file as a string. The screenshot is not saved to the disk.
--- @return string A binary string containing a PNG image.
function emu.takeScreenshot()
end

--- Removes a previously registered callback function.
--- @param reference integer Value returned by the call to emu.addEventCallback()
--- @param eventType eventType Event type
function emu.removeEventCallback(reference, eventType)
end

--- Removes a previously registered callback function.
--- @param reference integer Value returned by the call to emu.addMemoryCallback()
--- @param callbackType callbackType Callback type
--- @param startAddress integer Start of the address range
--- @param endAddress integer? End of the address range
--- @param cpuType cpuType? CPU used for the callback
--- @param memoryType memType? Memory type
function emu.removeMemoryCallback(reference, callbackType, startAddress, endAddress, cpuType, memoryType)
end

--- Writes an 8-bit value to the specified address and memory type.
---
--- Note: When using "memType.[cpuName]" memory types, side-effects can occur from writing a value. Use the "memType.[cpuName]Debug" enum values to avoid side-effects.
--- @param address integer Address to write to
--- @param value integer 8-bit value to write
--- @param memoryType memType Memory type to write to
function emu.write(address, value, memoryType)
end

--- Writes a 16-bit value to the specified address and memory type.
---
--- Note: When using "memType.[cpuName]" memory types, side-effects can occur from writing a value. Use the "memType.[cpuName]Debug" enum values to avoid side-effects.
--- @param address integer Address to write to
--- @param value integer 16-bit value to write
--- @param memoryType memType Memory type to write to
function emu.writeWord(address, value, memoryType)
end

return emu