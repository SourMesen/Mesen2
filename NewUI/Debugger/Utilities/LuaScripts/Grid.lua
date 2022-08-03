-----------------------
-- Name: Grid
-- Author: upsilandre
-----------------------
-- Displays a grid overlay on the screen (8x8, 16x16 or 32x32) - right-click to change the grid's size
-----------------------

function Main()
  if emu.getMouseState().right then
    if not hold then
      hold = true
      mode = (mode + 1) % 3
      emu.drawRectangle( 95, 87, 67, 11, 0x808080, false, 50)
      if mode == 0 then
        size = 8
        emu.drawString(96, 89, "  8x8 Grid  ", 0xFFFFFF, 0x404040, 0, 50)
      elseif mode == 1 then
        size = 16
        emu.drawString(96, 89, " 16x16 Grid ", 0xFFFFFF, 0x404040, 0, 50)
      else
        size = 32
        emu.drawString(96, 89, " 32x32 Grid ", 0xFFFFFF, 0x404040, 0, 50)
      end
    end
  else
    hold = false  
  end
  for i = 0, 239, size do
    emu.drawLine( 0, i, 255, i, color, 1)
  end
  for i = 0, 255, size do
    emu.drawLine( i, 0, i, 239, color, 1)
  end
end


color = 0xC0FF0000
hold = false
mode = 0
size = 8
emu.addEventCallback(Main, emu.eventType.endFrame);
emu.displayMessage("Script", "Grid")
