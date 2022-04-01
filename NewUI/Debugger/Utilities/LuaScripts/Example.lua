--This is an example script to give a general idea of how to build scripts
--Press F5 or click the Run button to execute it
--Scripts must be written in Lua (https://www.lua.org)
--This text editor contains an auto-complete feature for all Mesen-specific functions
--Typing "emu." will display a list containing every available API function to interact with Mesen

function printInfo()
  --Get the emulation state
  state = emu.getState()
  
  --Get the mouse's state (x, y, left, right, middle)
  mouseState = emu.getMouseState()  
  
  --Select colors based on whether the left button is held down
  if mouseState.left == true then
    buffer = emu.getScreenBuffer()
    for y = 0, 238 do
      for x = 0, 255 do
        buffer[y*256+x] = buffer[y*256+x] & 0xFFFF
      end
    end
    emu.setScreenBuffer(buffer)

    bgColor = 0x30FF6020
    fgColor = 0x304040FF
  else 
    bgColor = 0x302060FF
    fgColor = 0x30FF4040
  end
  
  --Draw some rectangles and print some text
  emu.drawRectangle(8, 8, 128, 24, bgColor, true, 1)
  emu.drawRectangle(8, 8, 128, 24, fgColor, false, 1)
  emu.drawString(12, 12, "Frame: " .. state.ppu.frameCount, 0xFFFFFF, 0xFF000000, 1)
  emu.drawString(12, 21, "CPU Cycle: " .. state.cpu.cycleCount, 0xFFFFFF, 0xFF000000, 1)
  
  emu.drawRectangle(8, 218, 193, 11, bgColor, true, 1)
  emu.drawRectangle(8, 218, 193, 11, fgColor, false, 1)  
  emu.drawString(11, 220, "Hold left mouse button to switch colors", 0xFFFFFF, 0xFF000000, 1)
  
  --Draw a block behind the mouse cursor - leaves a trail when moving the mouse
  emu.drawRectangle(mouseState.x - 2, mouseState.y - 2, 5, 5, 0xAF00FF90, true, 20)
  emu.drawRectangle(mouseState.x - 2, mouseState.y - 2, 5, 5, 0xAF000000, false, 20)
end

--Register some code (printInfo function) that will be run at the end of each frame
emu.addEventCallback(printInfo, emu.eventType.endFrame);

--Display a startup message
emu.displayMessage("Script", "Example Lua script loaded.")