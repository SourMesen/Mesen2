-----------------------
-- Name: Modify Screen
-- Author: spiiin
-----------------------
-- Alters the screen buffer to create various video filters
-- Press 0 to 4 to select a filter
-----------------------

shaderType = 0
frameCount = 0

function getCurrentFrame()
  frameCount = frameCount + 1
  return frameCount
end

function makeRed()
  local buffer = emu.getScreenBuffer()
  for i = 1, #buffer do
    buffer[i] = buffer[i] & 0xFFFF0000
  end
  emu.setScreenBuffer(buffer)
end

function onlyOddLines()
  local size = emu.getScreenSize()  
  local buffer = emu.getScreenBuffer()
  for i = 0, size.height - 1 do
    if i % 2 == 0 then
      local start = i * size.width
      for j = 1, size.width do
        buffer[start + j] = 0x00000000
      end
    end
  end
  emu.setScreenBuffer(buffer)
end

function blink()
   local buffer = emu.getScreenBuffer()
   local frame = getCurrentFrame()
   local blinkPeriod = 24*5
   local blinkTime = 10
   if frame % blinkPeriod < blinkTime then
     for i = 1, #buffer do
       buffer[i] = 0x0000FF00
     end
   end
   emu.setScreenBuffer(buffer)
end

function waves()
  local frame = getCurrentFrame()
  local maxWave = 40
  local waveSpeed = 4
  local anim = (frame // waveSpeed % maxWave)
  local animTable = {
    -10,-9,-8,-7,-6,-5,-4,-3,-2,-1,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
     10, 9, 8, 7, 6, 5, 4, 3, 2, 1,
      0,-1,-2,-3,-4,-5,-6,-7,-8,-9,
  }
  anim = animTable[anim+1]
  local width = emu.getScreenSize().width
  local buffer = emu.getScreenBuffer()
  local buffer2 = emu.getScreenBuffer()
  for i = 1, #buffer do
    local x = i % width
    local y = i // width
    if y % 2 == 0 then
      buffer[i] = buffer2[i+anim]
    else
      buffer[i] = buffer2[i-anim]
    end
  end
  emu.setScreenBuffer(buffer)
end

function endFrame()
  if shaderType == 1 then
    makeRed()
  elseif shaderType == 2 then
    onlyOddLines()
  elseif shaderType == 3 then
    blink()
  elseif shaderType == 4 then
    waves()
  end
end

function checkKeys()
  if emu.isKeyPressed("0") and shaderType ~= 0 then
    shaderType = 0
    emu.displayMessage("Info", "No shader selected")
  elseif emu.isKeyPressed("1") and shaderType ~= 1 then
    shaderType = 1
    emu.displayMessage("Info", "Shader Red selected")
  elseif emu.isKeyPressed("2") and shaderType ~= 2 then
    shaderType = 2
    emu.displayMessage("Info", "Shader OddLines selected")
  elseif emu.isKeyPressed("3") and shaderType ~= 3 then
    shaderType = 3
    emu.displayMessage("Info", "Shader Blink selected")
  elseif emu.isKeyPressed("4") and shaderType ~= 4 then
    shaderType = 4
    emu.displayMessage("Info", "Shader Waves selected")
  end
end

emu.addEventCallback(endFrame, emu.eventType.endFrame)
emu.addEventCallback(checkKeys, emu.eventType.inputPolled)

local infoStr = "Press keys from 0 to 4 to change shaders"
emu.displayMessage("Info", infoStr)
emu.log(infoStr)
