-----------------------
-- Name: Log Parallax
-- Author: spiiin
-----------------------
-- Draws a red line over each scanline that CPU writes to $2005 occurred
-----------------------

local consoleType = emu.getState()["consoleType"]
if consoleType ~= "Nes" then
  emu.displayMessage("Script", "This script only works on the NES.")
  return
end

PPUSCROLL = 0x2005
colorCode = 0x4000FF00

function onScroll(address, value)
  local scanline = emu.getState()["ppu.scanline"]
  if scanline < 240 and scanline >= 0 then
    emu.log("Scrolling change. Scanline: "..scanline.." Value:"..value)
    local color = colorCode + scanline
    emu.drawLine(0, scanline, 256, scanline, color, 1)
  end
end

emu.addMemoryCallback(onScroll, emu.callbackType.write, PPUSCROLL)
emu.displayMessage("Script", "Log Parallax")
