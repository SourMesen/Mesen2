-----------------------
-- Name: Display State 
-- Author: Sour
-----------------------
-- Displays a sorted list of all values returned by emu.getState()
-- The result is shown in the log window below the script after executing it.
-- The values returned vary by game and console.
--
-- Note: Some values represent the emulator's internal state and
--       may be added/changed/removed in future versions.
--
-- The values can be accessed by using, e.g:
--     local state = emu.getState()
--     state["masterClock"]
--     state["region"]
--     state["cpu.pc"]
--     etc.
-----------------------

local state = emu.getState()

local keys={}
local n=0
for k,v in pairs(state) do
  n=n+1
  keys[n]=k
end

table.sort(keys)

local output = ''
for i = 1, #keys do
 output = output .. keys[i] ..': ' .. tostring(state[keys[i]]) .. '\r'
end

emu.log(output)
