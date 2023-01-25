-----------------------
-- Name: DMC Capture
-- Author: upsilandre
-----------------------
-- Displays information about the DMC samples: [Sample Address] [Sample Size (bytes)] [Sample Frequency]
-- Every new sample encountered will be logged in the script's log window.
-- Right-clicking on the screen turns on DMC sample recording. While recording, any sample played
-- will be recorded on the disk in .wav format in the LuaScriptData folder
-----------------------

local consoleType = emu.getState()["consoleType"]
if consoleType ~= "Nes" then
  emu.displayMessage("Script", "This script only works on the NES.")
  return
end

function Main()
  local state = emu.getState()
  local dmc = {
	  bytesRemaining = state["apu.dmc.bytesRemaining"],
	  sampleAddr = state["apu.dmc.sampleAddr"],
	  sampleLength = state["apu.dmc.sampleLength"],
	  period = state["apu.dmc.timer.period"]
  }
  
  dmc.sampleRate = state["clockRate"] / (dmc.period + 1)   

  SelectRecord()
  if dmc.bytesRemaining > 0 then
    freq = math.floor(dmc.sampleRate)
    new = true
    if nbSample > 0 then
      for i = 1,nbSample do
        if (dmc.sampleAddr == sample.addr[i] and dmc.sampleLength == sample.length[i] and dmc.period == sample.period[i]) then
          new = false
          break
        end
      end
    end
    if new then
      nbSample = nbSample + 1
      sample.addr[nbSample] = dmc.sampleAddr
      sample.length[nbSample] = dmc.sampleLength
      sample.period[nbSample] = dmc.period
      emu.log(nbSample.." >>  $"..string.format("%X  ", dmc.sampleAddr)..string.format("%4d bytes  ",dmc.sampleLength)..string.format("%5d hertz ",freq)..stringR)
      if record then
        SampleRecord(dmc.sampleAddr, dmc.sampleLength, freq)
      end
    end
    if dmc.sampleAddr ~= lastSampleAddr or dmc.sampleLength ~= lastSampleLength or dmc.period ~= lastSamplePeriod then
      lastSampleAddr = dmc.sampleAddr
      lastSampleLength = dmc.sampleLength
      lastSamplePeriod = dmc.period
      emu.drawString(4, 7, " $"..string.format("%X  ", dmc.sampleAddr)..string.format("%4d bytes  ",dmc.sampleLength)..string.format("%5d hertz ",freq), 0xffffff, 0x40ffffff)
    else
      emu.drawString(4, 7, " $"..string.format("%X  ", dmc.sampleAddr)..string.format("%4d bytes  ",dmc.sampleLength)..string.format("%5d hertz ",freq), 0xc0c0c0, 0x40202020)
    end
  end
end


function SelectRecord()
  if emu.getMouseState().right then
    if not hold then
      hold = true
      
      if not io then 
        emu.displayMessage("Script", "I/O access is disabled in the options")
        return
      end
      
      record = not record
      nbSample = 0
      emu.log("______________________________________________________________________\n")
      if record then
        emu.log("File path = "..emu.getScriptDataFolder().."\\\n")
      end
    end
  else
    hold = false
  end
  if record then
    stringR = "  recorded"
    emu.drawRectangle(193, 5, 42, 11, 0x404040, true, 1)
    emu.drawRectangle(192, 4, 44, 13, 0x808080, false, 1)
    emu.drawString(196, 7, "RECORD", 0xFF0000, 0xFF000000)
  else
    stringR = ""
  end
end


function SampleRecord(addr, length, freq)
  dataLen = length * 8 + 1
  header[5] = (dataLen + 36) & 255
  header[6] = (dataLen + 36) >> 8
  header[25] = freq & 255
  header[26] = freq >> 8
  header[29] = header[25]
  header[30] = header[26]
  header[41] = dataLen & 255
  header[42] = dataLen >> 8
  header[45] = 0x80
  fileName = string.sub(emu.getRomInfo().name, 1, #emu.getRomInfo().name - 4)..string.format("_$%X_", addr)..length.."b_"..freq.."hz.wav"
  fileOutput = io.open(emu.getScriptDataFolder().."\\"..fileName, "w+b")
  for k = 1,45 do
    fileOutput:write(string.char(header[k]))
  end
  lastSample = 0x80
  for i = 0, length - 1 do
    bitMask = 1
    byte = emu.read(addr + i, emu.memType.nesDebug)
    for j = 1,8 do
      if byte & bitMask > 0 then
        newSample = lastSample + 4
        if newSample == 256 then
          newSample = 252
        end
      else
        newSample = lastSample - 4
        if newSample == -4 then
          newSample = 0
        end
      end
      fileOutput:write(string.char(newSample))
      lastSample = newSample
      bitMask = bitMask << 1
    end
  end
  fileOutput:close()
end


sample = {}
sample.addr = {}
sample.length = {}
sample.period = {}
nbSample = 0
record = false
hold = false
header = {0x52,0x49,0x46,0x46,0x00,0x00,0x00,0x00,
          0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,
          0x10,0x00,0x00,0x00,0x01,0x00,0x01,0x00,
          0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
          0x01,0x00,0x08,0x00,0x64,0x61,0x74,0x61,
          0x00,0x00,0x00,0x00}
emu.addEventCallback(Main, emu.eventType.endFrame)
emu.displayMessage("Script", "DMC Capture")
