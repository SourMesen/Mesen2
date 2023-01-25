-----------------------
-- Name: Game Boy Mode
-- Author: upsilandre
-----------------------
-- Simulates the game boy's display limitations
-- 1. Adds a 160x144 frame which can be moved by clicking inside it.  Clicking outside of it will hide the black overlay.
-- 2. Restricts the palette to a 4-color palette like the game boy - right-click the screen to change the palette.
-----------------------

local consoleType = emu.getState()["consoleType"]
if consoleType ~= "Nes" then
  emu.displayMessage("Script", "This script only works on the NES.")
  return
end

function Main()
  mouse = emu.getMouseState()
  if mouse.left then
    if stencil then
      if mouse.x > window_X and mouse.x < window_X + 160 and mouse.y > window_Y and mouse.y < window_Y + 144 then
        window_X = mouse.x - 80
        window_Y = mouse.y - 72
      else
        stencil = false
        holdLeft = true
      end
    else
      if not holdLeft then
        holdLeft = true
        stencil = true
        window_X = mouse.x - 80
        window_Y = mouse.y - 72
      end
    end
  else
    holdLeft = false
  end

  if stencil then
    if window_X > 0 then
      emu.drawRectangle(0, window_Y, window_X, 144, 0x000000, true)
    end
    if window_X < 96 then
      emu.drawRectangle(window_X + 160, window_Y, 96 - window_X, 144, 0x000000, true)
    end
    if window_Y > 0 then
      emu.drawRectangle(0, 0, 256, window_Y, 0x000000, true)
    end
    if window_Y < 96 then
    emu.drawRectangle(0, window_Y + 144, 256, 96 - window_Y, 0x000000, true)
    end
  end

  if mouse.right then
    if not holdRight then
      holdRight = true
      UI_X = mouse.x - selectX
      UI_Y = mouse.y - selectY
    end
    emu.drawRectangle(UI_X+16, UI_Y, 128, 16, 0x808080, true)
    emu.drawRectangle(UI_X, UI_Y+16, 16, 128, 0x808080, true)
    for x = UI_X+16, UI_X+128, 16 do
      emu.drawRectangle(x, UI_Y, 17, 17, 0x404040)
    end
    for y = UI_Y+16, UI_Y+128, 16 do
      emu.drawRectangle(UI_X, y, 17, 17, 0x404040)
    end
    if mouse.x > UI_X + 16 and mouse.x < UI_X + 145 and  mouse.y > UI_Y + 16 and mouse.y < UI_Y + 145 then
      paletteBg = (mouse.x - 1 - UI_X) >> 4
      paletteSpr = (mouse.y - 1 - UI_Y) >> 4
      offsetX = (UI_X & 0x0f) + 1
      offsetY = (UI_Y & 0x0f) + 1
      emu.drawRectangle(((mouse.x - offsetX) & 0xf0) + offsetX, UI_Y+1, 15, 15, 0xff0000, true)
      emu.drawRectangle(UI_X+1, ((mouse.y - offsetY) & 0xf0) + offsetY, 15, 15, 0xff0000, true)
      emu.drawLine(UI_X+17, mouse.y, mouse.x, mouse.y, 0xff0000)
      emu.drawLine(mouse.x, UI_Y+17, mouse.x, mouse.y, 0xff0000)
      selectX = mouse.x - UI_X
      selectY = mouse.y - UI_Y
    end
    for i = 1,#text,3 do
      emu.drawString(UI_X + text[i], UI_Y + text[i+1], text[i+2], 0xffffff, 0xff000000)
    end
  else
    holdRight = false
  end

  for palAddr = 0,0x0f,4 do
    emu.write(palAddr, color[paletteBg][1], emu.memType.nesPaletteRam)
    emu.write(palAddr + 1, color[paletteBg][2], emu.memType.nesPaletteRam)
    emu.write(palAddr + 2, color[paletteBg][3], emu.memType.nesPaletteRam)
    emu.write(palAddr + 3, color[paletteBg][4], emu.memType.nesPaletteRam)
    emu.write(palAddr + 16, color[paletteBg][1], emu.memType.nesPaletteRam)
    emu.write(palAddr + 17, color[paletteSpr][1], emu.memType.nesPaletteRam)
    emu.write(palAddr + 18, color[paletteSpr][2], emu.memType.nesPaletteRam)
    emu.write(palAddr + 19, color[paletteSpr][3], emu.memType.nesPaletteRam)
  end
end

stencil = false
holdLeft = false
holdRight = false
window_X = 48
window_Y = 48
selectX = 72
selectY = 72
paletteSpr = 4
paletteBg = 4
color = {{0x38,0x28,0x18,0x08},{0x28,0x18,0x08,0x38},{0x18,0x08,0x38,0x28},{0x08,0x38,0x28,0x18},
{0x08,0x18,0x28,0x38},{0x18,0x28,0x38,0x08},{0x28,0x38,0x08,0x18},{0x38,0x08,0x18,0x28}}
text = {48,-9,"Background",22,5,"1",38,5,"2",54,5,"3",70,5,"4",86,5,"5",102,5,"6",
118,5,"7",134,5,"8",-6,51,"S",-6,57,"p",-6,64,"r",-6,72,"i",-7,80,"t",-7,86,"e",
6,21,"1",6,37,"2",6,53,"3",6,69,"4",6,85,"5",6,101,"6",6,117,"7",6,133,"8"}
emu.addEventCallback(Main, emu.eventType.startFrame)
emu.displayMessage("Script", "Game Boy mode")
