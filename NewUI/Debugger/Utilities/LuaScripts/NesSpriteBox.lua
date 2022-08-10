-----------------------
-- Name: Sprite Box
-- Author: upsilandre
-----------------------
-- Displays a box around each sprite, as well as a sprite counter.
-- The counter displays the number of sprites on the screen (and will be shown in red if sprite overflow occurred on any scanline)
--
-- Each scanline with overflow will be marked by a small red line on the left side of the screen.
-- As the number of sprites over the limit increases, the line will progressively turn orange, yellow and then white.
--
-- Sprites with a red box as normal priority sprites, those in blue are background priority sprites.
--
-- 4 different display modes exist (change mode by right-clicking on the screen):
--   1- Edge Mode: Displays a rectangle outline around the sprites
--   2- Fill Mode: Displays a transparent overlay over the sprites
--   3- Priority Mode: Displays a transparent overlay over the sprites (the overlay's opacity is proportional to the sprite's position in OAM RAM)
--   4- Off Mode: Displays the sprite counter and nothing else
-----------------------

local consoleType = emu.getState()["consoleType"]
if consoleType ~= "Nes" then
  emu.displayMessage("Script", "This script only works on the NES.")
  return
end

function Main()
  SelectMode()
  spritesOnScreen = 0
  counterColor = 0xFFFFFF
  for scanline = 0,254 do
    spritesOnLine[scanline] = 0
  end

  if emu.getState()["ppu.control.largeSprites"] then
    height = 16
  else
    height = 8
  end

  for oamAddr = 0, 252, 4 do
    spriteY = emu.read(oamAddr, emu.memType.nesSpriteRam) + 1
    if spriteY < 240 then
      spritesOnScreen = spritesOnScreen + 1
      for i = 0, (height - 1 ) do
        spritesOnLine[spriteY + i] = spritesOnLine[spriteY + i] + 1
      end
    end
    spriteX = emu.read(oamAddr + 3, emu.memType.nesSpriteRam)
    if emu.read(oamAddr + 2, emu.memType.nesSpriteRam) & 32 == 0 then
      color = 0xff0000
    else
      color = 0x0000ff
    end
    if mode == 2 then
      alpha = oamAddr / 4 * 0x03000000 + 0x20000000
    end
    emu.drawRectangle(spriteX, spriteY, 8, height, color + alpha, fill, 1)
  end

  for scanline = 0,239 do
    overflow = spritesOnLine[scanline] - 8
    if overflow > 0 then
      if overflow > 16 then
        overflowColor = 0xFFFFFF
      elseif overflow > 8 then
        overflowColor = 0xFFFF00 + (((overflow - 1) & 7) << 5)
      else
        overflowColor = 0xFF0000 + (((overflow - 1) & 7) << 13)
      end
      emu.drawLine(0, scanline, 7, scanline, overflowColor, 1)
      counterColor = 0xFF0000
    end
  end

  emu.drawRectangle(121, 3, 13, 9, 0x404040, true, 1)
  emu.drawRectangle(120, 2, 15, 11, 0x808080, false, 1)
  emu.drawString(122, 4, string.format("%02d", spritesOnScreen), counterColor, 0xFF000000)
end


function SelectMode()
  if emu.getMouseState().right then
    if not holdButton then
      mode = (mode + 1) & 3
      emu.drawRectangle(94, 87, 68, 12, 0x404040, true, 50)
      emu.drawRectangle(93, 86, 70, 14, 0x808080, false, 50)
      if mode == 0 then
        fill = false
        alpha = 0x40000000
        emu.drawString(92, 89, "  Edge Mode  ", 0xFFFFFF, 0xFF000000, 0, 50)
      elseif mode == 1 then
        fill = true
        alpha = 0x80000000
        emu.drawString(92, 89, "   Fill Mode   ", 0xFFFFFF, 0xFF000000, 0, 50)
      elseif mode == 2 then
        emu.drawString(92, 89, " Priority Mode ", 0xFFFFFF, 0xFF000000, 0, 50)
      else
        emu.drawString(92, 89, "  OFF Mode  ", 0xFFFFFF, 0xFF000000, 0, 50)
        alpha =0xFF000000
      end
      holdButton = true
    end
  else
    holdButton = false
  end
end


mode = 0
alpha = 0x40000000
holdButton = false
fill = false
spritesOnLine = {}
emu.addEventCallback(Main, emu.eventType.startFrame)
emu.displayMessage("Script", "Sprite Box")
