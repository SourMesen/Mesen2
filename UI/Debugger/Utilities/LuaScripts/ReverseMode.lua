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
  bufferI = emu.getScreenBuffer()
  for y = 0, 239 do
    local offset = y*256;
    for x = 0, 255 do
      bufferO[offset + x] = bufferI[offset + 255 - x]
    end
  end
  emu.setScreenBuffer(bufferO)
end

emu.addEventCallback(Main, emu.eventType.inputPolled)
emu.displayMessage("Script", "Reverse Mode")
