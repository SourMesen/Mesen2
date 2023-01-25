-----------------------
-- Name: Draw Mode
-- Author: upsilandre
-----------------------
-- Allows you to draw an overlay over the game's screen by left-clicking with the mouse.
-- Right-click to clear the overlay.
-----------------------

function Main()
  mouse = emu.getMouseState()
  if mouse.left then
    if not hold then
      hold = true
    else
      emu.drawLine( oldX, oldY, mouse.x, mouse.y, 0xFF0000, 10000000)
    end
    oldX = mouse.x
    oldY = mouse.y
  else
    hold = false
  end  
  if mouse.right then
    emu.clearScreen()
  end
end


hold = false
emu.addEventCallback(Main, emu.eventType.endFrame);
emu.displayMessage("Script", "Draw Mode")
