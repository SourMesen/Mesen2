-----------------------
-- Name: NTSC Safe Area
-- Author: Sour
-----------------------
-- Displays an overlay based on tepples' "Safe Area on 5.37Mhz NTSC VDPs" diagram
-- Source: https://wiki.nesdev.com/w/index.php/File:Safe_areas.png
--
-- Red: "Danger Zone" (Top and bottom 8 pixels)
--      Most TVs hide all of this.
--
-- Gray: Outside the "Title Safe Area". 
--       Some TVs may hide portions of this.
--       Text, etc. should not appear here.
-----------------------

function drawSafeArea()
  local red = 0x40FF0000;
  local gray = 0x60505050;

  emu.drawRectangle(0, 0, 256, 8, red, true)
  emu.drawRectangle(0, 232, 256, 8, red, true)
  
  emu.drawRectangle(0, 8, 16, 224, gray, true)
  emu.drawRectangle(240, 8, 16, 224, gray, true)
  emu.drawRectangle(16, 8, 224, 16, gray, true)
  emu.drawRectangle(16, 216, 224, 16, gray, true)
end

emu.addEventCallback(drawSafeArea, emu.eventType.endFrame);
