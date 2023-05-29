-----------------------
-- Name: Reverse Mode
-- Author: upsilandre
-----------------------
-- Flips the game screen horizontally and inverts the left & right buttons
-- on the controller, allowing you to play through games in reverse.
-----------------------

bufferO = {}
function Main()
  input = emu.getInput(0)
  input.left, input.right = input.right, input.left
  emu.setInput(input, 0)
  local size = emu.getScreenSize()
  bufferI = emu.getScreenBuffer()
  for y = 0, size.height do
    local offset = y*size.width;
    for x = 1, size.width do
      bufferO[offset + x] = bufferI[offset + size.width - x + 1]
    end
  end
  emu.setScreenBuffer(bufferO)
end

emu.addEventCallback(Main, emu.eventType.inputPolled)
emu.displayMessage("Script", "Reverse Mode")
