-----------------------
-- Name: Piano Roll (NES)
-- Author: zeta0134
-- Based on: https://github.com/zeta0134/mesen-piano-roll
-----------------------

local consoleType = emu.getState()["consoleType"]
if consoleType ~= "Nes" then
  emu.displayMessage("Script", "This script only works on the NES.")
  return
end

local NTSC_CPU_FREQUENCY = 1.789773 * 1024.0 * 1024.0
local LOWEST_NOTE_FREQ = 55.0 -- ~A0
local HIGHEST_NOTE_FREQ = 4434.922 -- ~C#8
local PIANO_ROLL_WIDTH = 240
local PIANO_ROLL_KEYS = 76
local PIANO_ROLL_KEY_HEIGHT = 2
local PIANO_ROLL_HEIGHT = PIANO_ROLL_KEYS * PIANO_ROLL_KEY_HEIGHT
local NOISE_KEY_HEIGHT = 2
local NOISE_ROLL_HEIGHT = NOISE_KEY_HEIGHT * 16
local NOISE_ROLL_OFFSET = 200
local SQUARE1_COLORS = {0xFFA0A0, 0xFF40FF, 0xFF4040}
SQUARE1_COLORS[4] = SQUARE1_COLORS[2]
local SQUARE2_COLORS = {0xFFE0A0, 0xFFC040, 0xFFFF40}
SQUARE2_COLORS[4] = SQUARE2_COLORS[2]
local TRIANGLE_COLORS = {0x40FF40}
local NOISE_COLORS = {0x4040FF, 0x40FFFF}
local DMC_BASE_COLOR = 0x806040A0
local DMC_PLAYING_COLOR = 0x8040C0
local DMC_OFFSET = 178
local DMC_HEIGHT = 19
local BACKGROUND_COLOR = 0x80000000
local PIANO_STRING_BLACK_COLOR = 0x80101010
local PIANO_STRING_WHITE_COLOR = 0x80060606
local NOISE_STRING_BLACK_COLOR = 0x80060606
local NOISE_STRING_WHITE_COLOR = 0x800A0A0A

local settings = {
  background="solid"
}

local shadow_apu = {}
for address = 0x4000, 0x4017 do
  shadow_apu[address] = 0
end

local shadow_triangle = {}
shadow_triangle.lc_reload = false
shadow_triangle.lc_value = 0

function clock_linear_counter()
  if shadow_triangle.lc_reload then
    shadow_triangle.lc_value = shadow_apu[0x4008] & 0x7F
  else
    if shadow_triangle.lc_value > 0 then
      shadow_triangle.lc_value = shadow_triangle.lc_value - 1
    end
  end
  if shadow_apu[0x4008] & 0x80 == 0 then
    shadow_triangle.lc_reload = false
  end
end

function clock_shadow_frame_sequencer()
  clock_linear_counter()
  clock_linear_counter()
  clock_linear_counter()
  clock_linear_counter()
end

function apu_register_write(address, value)
  shadow_apu[address] = value
  if address == 0x400B then
    shadow_triangle.lc_reload = true
  end
end

emu.addEventCallback(clock_shadow_frame_sequencer, emu.eventType.endFrame)
emu.addMemoryCallback(apu_register_write, emu.callbackType.write, 0x4000 , 0x4017)

function set_background()
  if settings.background == "solid" then
    BACKGROUND_COLOR = 0x000000
    PIANO_STRING_BLACK_COLOR = 0x101010
    PIANO_STRING_WHITE_COLOR = 0x060606
    NOISE_STRING_BLACK_COLOR = 0x060606
    NOISE_STRING_WHITE_COLOR = 0x0A0A0A
    DMC_BASE_COLOR = 0x006040A0
  end
  if settings.background == "transparent" then
    BACKGROUND_COLOR = 0x80000000
    PIANO_STRING_BLACK_COLOR = 0x80101010
    PIANO_STRING_WHITE_COLOR = 0x80060606
    NOISE_STRING_BLACK_COLOR = 0x80060606
    NOISE_STRING_WHITE_COLOR = 0x800A0A0A
    DMC_BASE_COLOR = 0x806040A0
  end
  if settings.background == "clear" then
    BACKGROUND_COLOR = 0xFF000000
    PIANO_STRING_BLACK_COLOR = 0xFF000000
    PIANO_STRING_WHITE_COLOR = 0xFF000000
    NOISE_STRING_BLACK_COLOR = 0xFF060606
    NOISE_STRING_WHITE_COLOR = 0xFF0A0A0A
    DMC_BASE_COLOR = 0x806040A0
  end
end
-- call this once to initialize
set_background()

function toggle_background()
  if settings.background == "clear" then
    settings.background = "transparent"
  elseif settings.background == "transparent" then
    settings.background = "solid"
  elseif settings.background == "solid" then
    settings.background = "clear"
  end
  set_background()
end

function tiny_a(x, y, color)
  emu.drawLine(x,y+1,x, y+4, color)
  emu.drawLine(x+2,y+1,x+2, y+4, color)
  emu.drawPixel(x+1,y,color)
  emu.drawPixel(x+1,y+2,color)
end

function tiny_b(x, y, color)
  emu.drawLine(x, y, x, y+4, color)
  emu.drawPixel(x+1, y, color)
  emu.drawPixel(x+1, y+2, color)
  emu.drawPixel(x+1, y+4, color)
  emu.drawPixel(x+2, y+1, color)
  emu.drawPixel(x+2, y+3, color)
end

function tiny_c(x, y, color)
  emu.drawLine(x, y+1, x, y+3, color)
  emu.drawLine(x+1, y, x+2, y, color)
  emu.drawLine(x+1, y+4, x+2, y+4, color)
end

function tiny_d(x, y, color)
  emu.drawLine(x, y, x, y+4, color)
  emu.drawPixel(x+1, y, color)
  emu.drawPixel(x+1, y+4, color)
  emu.drawLine(x+2, y+1, x+2, y+3,color)
end

function tiny_e(x, y, color)
  emu.drawLine(x, y, x, y+4, color)
  emu.drawLine(x+1, y, x+2, y, color)
  emu.drawPixel(x+1, y+2, color)
  emu.drawLine(x+1, y+4, x+2, y+4, color)
end

function tiny_f(x, y, color)
  emu.drawLine(x, y, x, y+4, color)
  emu.drawLine(x+1, y, x+2, y, color)
  emu.drawPixel(x+1, y+2, color)
end

function tiny_g(x, y, color)
  emu.drawLine(x, y+1, x, y+3, color)
  emu.drawLine(x+1, y, x+2, y, color)
  emu.drawLine(x+1, y+4, x+2, y+4, color)
  emu.drawLine(x+2, y+2, x+2, y+3, color)
end

function tiny_h(x, y, color)
  emu.drawLine(x, y, x, y+4, color)
  emu.drawLine(x+2, y, x+2, y+4, color)
  emu.drawPixel(x+1, y+2, color)
end

function tiny_i(x, y, color)
  emu.drawLine(x, y, x+2, y, color)
  emu.drawLine(x, y+4, x+2, y+4, color)
  emu.drawLine(x+1, y+1, x+1, y+3, color)
end

function tiny_j(x, y, color)
  emu.drawLine(x, y, x+2, y, color)
  emu.drawLine(x+2, y+1, x+2, y+3, color)
  emu.drawPixel(x+1, y+4, color)
  emu.drawPixel(x, y+3, color)
end

function tiny_k(x, y, color)
  emu.drawLine(x, y, x, y+4, color)
  emu.drawPixel(x+1, y+2, color)
  emu.drawLine(x+2, y, x+2, y+1, color)
  emu.drawLine(x+2, y+3, x+2, y+4, color)
end

function tiny_l(x, y, color)
  emu.drawLine(x, y, x, y+4, color)  
  emu.drawLine(x+1, y+4, x+2, y+4, color)
end

function tiny_m(x, y, color)
  emu.drawLine(x, y, x, y+4, color)  
  emu.drawLine(x+1, y+1, x+1, y+2, color)  
  emu.drawLine(x+2, y, x+2, y+4, color)  
end

function tiny_n(x, y, color)
  emu.drawLine(x, y, x, y+4, color)  
  emu.drawPixel(x+1, y, color)  
  emu.drawLine(x+2, y+1, x+2, y+4, color)  
end

function tiny_o(x, y, color)
  emu.drawLine(x, y+1, x, y+3, color)
  emu.drawLine(x+2, y+1, x+2, y+3, color)
  emu.drawPixel(x+1, y, color)
  emu.drawPixel(x+1, y+4, color)
end

function tiny_p(x, y, color)
  emu.drawLine(x, y, x, y+4, color)  
  emu.drawPixel(x+1, y, color)
  emu.drawPixel(x+2, y+1, color)
  emu.drawPixel(x+1, y+2, color)
end

function tiny_q(x, y, color)
  emu.drawLine(x, y+1, x, y+3, color)
  emu.drawLine(x+2, y+1, x+2, y+3, color)
  emu.drawPixel(x+1, y, color)
  emu.drawPixel(x+1, y+3, color)
  emu.drawLine(x+1, y+4, x+2, y+4, color)
end

function tiny_r(x, y, color)
  emu.drawLine(x, y, x, y+4, color)  
  emu.drawPixel(x+1, y, color)
  emu.drawPixel(x+2, y+1, color)
  emu.drawPixel(x+1, y+2, color)
  emu.drawLine(x+2, y+3, x+2, y+4, color)  
end

function tiny_s(x, y, color)
  emu.drawLine(x+1, y, x+2, y, color)  
  emu.drawPixel(x, y+1, color)
  emu.drawPixel(x+1, y+2, color)
  emu.drawPixel(x+2, y+3, color)
  emu.drawLine(x, y+4, x+1, y+4, color)  
end

function tiny_t(x, y, color)
  emu.drawLine(x, y, x+2, y, color)  
  emu.drawLine(x+1, y+1, x+1, y+4, color)  
end

function tiny_u(x, y, color)
  emu.drawLine(x, y, x, y+4, color)  
  emu.drawLine(x+2, y, x+2, y+4, color)  
  emu.drawPixel(x+1, y+4, color)
end

function tiny_v(x, y, color)
  emu.drawLine(x, y, x, y+3, color)  
  emu.drawLine(x+2, y, x+2, y+3, color)  
  emu.drawPixel(x+1, y+4, color)
end

function tiny_w(x, y, color)
  emu.drawLine(x, y, x, y+4, color)  
  emu.drawLine(x+1, y+2, x+1, y+3, color)  
  emu.drawLine(x+2, y, x+2, y+4, color)  
end

function tiny_x(x, y, color)
  emu.drawLine(x, y, x, y+1, color)  
  emu.drawLine(x+2, y, x+2, y+1, color)  
  emu.drawPixel(x+1, y+2, color)
  emu.drawLine(x, y+3, x, y+4, color)  
  emu.drawLine(x+2, y+3, x+2, y+4, color)  
end

function tiny_y(x, y, color)
  emu.drawLine(x, y, x, y+2, color)  
  emu.drawLine(x+2, y, x+2, y+2, color)  
  emu.drawLine(x+1, y+2, x+1, y+4, color)
end

function tiny_z(x, y, color)
  emu.drawLine(x, y, x+2, y, color)
  emu.drawLine(x+2,y+1,x,y+3, color)
  emu.drawLine(x,y+4,x+2,y+4, color)
end

function tiny_0(x, y, color)
  emu.drawLine(x, y, x, y+4, color)
  emu.drawLine(x+2, y, x+2, y+4, color)
  emu.drawPixel(x+1, y, color)
  emu.drawPixel(x+1, y+4, color)
end

function tiny_1(x, y, color)
  emu.drawLine(x+1, y, x+1, y+4, color)
  emu.drawLine(x, y+4, x+2, y+4, color)
  emu.drawPixel(x, y+1,color)
end

function tiny_2(x, y, color)
  emu.drawLine(x, y, x+1, y, color)
  emu.drawLine(x+2,y+1,x,y+3, color)
  emu.drawLine(x,y+4,x+2,y+4, color)
end

function tiny_3(x, y, color)
  emu.drawLine(x+2, y, x+2, y+4, color)
  emu.drawLine(x, y, x+1, y, color)
  emu.drawPixel(x+1, y+2, color)
  emu.drawLine(x, y+4, x+1, y+4, color)
end

function tiny_4(x, y, color)
  emu.drawLine(x+2, y, x+2, y+4, color)
  emu.drawLine(x, y, x, y+2, color)
  emu.drawPixel(x+1, y+2, color)
end

function tiny_5(x, y, color)
  emu.drawLine(x, y, x+2, y, color)
  emu.drawLine(x, y+1, x, y+2, color)
  emu.drawPixel(x+1, y+2, color)
  emu.drawPixel(x+2, y+3, color)
  emu.drawLine(x,y+4,x+1,y+4, color)
end

function tiny_6(x, y, color)
  emu.drawLine(x, y, x, y+4, color)
  emu.drawLine(x+1, y, x+2, y, color)
  emu.drawLine(x+1, y+2, x+2, y+2, color)
  emu.drawLine(x+1, y+4, x+2, y+4, color)
  emu.drawPixel(x+2, y+3, color)
end

function tiny_7(x, y, color)
  emu.drawLine(x+2, y, x+2, y+4, color)
  emu.drawLine(x, y, x+1, y, color)
end

function tiny_8(x, y, color)
  emu.drawLine(x, y, x, y+4, color)
  emu.drawLine(x+2, y, x+2, y+4, color)
  emu.drawPixel(x+1, y, color)
  emu.drawPixel(x+1, y+2, color)
  emu.drawPixel(x+1, y+4, color)
end

function tiny_9(x, y, color)
  emu.drawLine(x+2, y, x+2, y+4, color)
  emu.drawLine(x, y, x+1, y, color)
  emu.drawLine(x, y+2, x+1, y+2, color)
  emu.drawLine(x, y+4, x+1, y+4, color)
  emu.drawPixel(x, y+1, color)
end

function tiny_hex_char(x, y, value, color)
  local hex_functions = {
    tiny_0, tiny_1, tiny_2, tiny_3, tiny_4, tiny_5, tiny_6, tiny_7,
    tiny_8, tiny_9, tiny_a, tiny_b, tiny_c, tiny_d, tiny_e, tiny_f}
  if hex_functions[value+1] then
    hex_functions[value+1](x,y,color)
  end
end

function tiny_hex(x, y, value, color, width)
  while width > 0 do
    width = width - 1
    tiny_hex_char(x + width * 4, y, value & 0xF, color)
    value = value >> 4
  end
end

local char_functions = {
  tiny_a, tiny_b, tiny_c, tiny_d, tiny_e, tiny_f,
  tiny_g, tiny_h, tiny_i, tiny_j, tiny_k, tiny_l,
  tiny_m, tiny_n, tiny_o, tiny_p, tiny_q, tiny_r,
  tiny_s, tiny_t, tiny_u, tiny_v, tiny_w, tiny_x,
  tiny_y, tiny_z
}

local num_functions = {
  tiny_0, tiny_1, tiny_2, tiny_3, tiny_4,
  tiny_5, tiny_6, tiny_7, tiny_8, tiny_9
}

function tiny_char(x, y, character_code, color)
  if character_code >= string.byte("a", 1) and character_code <= string.byte("z") then
    char_functions[character_code - string.byte("a") + 1](x, y, color)
  end
  if character_code >= string.byte("A", 1) and character_code <= string.byte("Z") then
    char_functions[character_code - string.byte("A") + 1](x, y, color)
  end
  if character_code >= string.byte("0", 1) and character_code <= string.byte("9") then
    num_functions[character_code - string.byte("0") + 1](x, y, color)
  end
end

function tiny_string(x, y, str, color)
  for i = 1, #str do
    tiny_char(x + ((i-1) * 4), y, string.byte(str, i), color)
  end
end

function frequency_to_coordinate(note_frequency, lowest_freq, highest_freq, viewport_height)
  local highest_log = math.log(highest_freq)
  local lowest_log = math.log(lowest_freq)
  local range = highest_log - lowest_log
  local note_log = math.log(note_frequency)

  local coordinate = (note_log - lowest_log) * viewport_height / range
  if coordinate >= 0 and coordinate < viewport_height then
    return coordinate
  else
    return nil
  end
end

function pulse_frequency(pulse_period)
  return NTSC_CPU_FREQUENCY / (16.0 * (pulse_period + 1.0))
end

function triangle_frequency(triangle_period)
  return NTSC_CPU_FREQUENCY / (32.0 * (triangle_period + 1.0))
end

function unpack_argb(color)
  return ((color & 0xFF000000) >> 24), ((color & 0xFF0000) >> 16), ((color & 0xFF00) >> 8), (color & 0xFF)
end

function pack_argb(a, r, g, b)
  return (a << 24) | (r << 16) | (g << 8) | (b)
end

function apply_brightness(color, brightness)
  local a, r, g, b = unpack_argb(color)
  r = math.floor(r * brightness)
  g = math.floor(g * brightness)
  b = math.floor(b * brightness)
  return pack_argb(a, r, g, b)
end

function apply_transparency(color, alpha)
  local a, r, g, b = unpack_argb(color)
  a = math.floor(0xFF * (1.0 - alpha))
  return pack_argb(a, r, g, b)
end

local square1_roll = {}
local square2_roll = {}
local triangle_roll = {}
local noise_roll = {}
local dmc_roll = {}

function initialize_piano_roll()
  for i = 1, PIANO_ROLL_WIDTH - 1 do
    local channel_state = {}
    -- this is enough for pulse, triangle, and noise
    channel_state.enabled = false
    -- dmc calculates a delta, so it needs full dummy samples
    channel_state.playing = false
    channel_state.delta = 0

    table.insert(square1_roll, channel_state)
    table.insert(square2_roll, channel_state)
    table.insert(triangle_roll, channel_state)
    table.insert(noise_roll, channel_state)
    table.insert(dmc_roll, channel_state)
  end
end

initialize_piano_roll()

function update_piano_roll(channel, state_table)
  local channel_state = {}
  local coordinate = frequency_to_coordinate(channel.frequency, LOWEST_NOTE_FREQ, HIGHEST_NOTE_FREQ, PIANO_ROLL_HEIGHT)
  if coordinate then
    channel_state.y = PIANO_ROLL_HEIGHT - coordinate
  end
  if channel.envelope then
    -- pulse channels
    channel_state.volume = channel.envelope.volume
    channel_state.enabled = channel.envelope.volume ~= 0 and channel.lengthCounter.counter > 0
    channel_state.duty = channel.duty
  else
    -- triangle channels
    channel_state.volume = 6
    channel_state.enabled = 
      channel.lengthCounter.counter > 0 and
      shadow_triangle.lc_value > 0 and
      channel.period > 2 and
      channel.enabled
    channel_state.duty = 0
  end
  
  table.insert(state_table, channel_state)
  if #state_table > PIANO_ROLL_WIDTH then
    table.remove(state_table, 1)
  end
end

local noise_period_table = {}
noise_period_table[4]    =  0 
noise_period_table[8]    =  1
noise_period_table[16]   =  2
noise_period_table[32]   =  3
noise_period_table[64]   =  4
noise_period_table[96]   =  5
noise_period_table[128]  =  6
noise_period_table[160]  =  7
noise_period_table[202]  =  8
noise_period_table[254]  =  9
noise_period_table[380]  =  10
noise_period_table[508]  =  11
noise_period_table[762]  =  12
noise_period_table[1016] =  13
noise_period_table[2034] =  14
noise_period_table[4068] =  15

function debug_table_keys(name, t)
  local keys = {}
  for k,v in pairs(t) do
    keys[#keys+1] = k
  end
  emu.log(name..": "..table.concat(keys, ","))
end

function update_noise_roll(channel, state_table)
  local channel_state = {}
  if noise_period_table[channel.period + 1] then
    channel_state.y = noise_period_table[channel.period + 1] * NOISE_KEY_HEIGHT
    channel_state.period = noise_period_table[channel.period + 1]
    channel_state.mode = channel.modeFlag
  end
  if channel.envelope.constantVolume then
    channel_state.volume = channel.envelope.volume
  else 
    channel_state.volume = channel.envelope.counter
  end
  channel_state.enabled = channel_state.volume ~= 0 and channel.lengthCounter.counter > 0

  table.insert(state_table, channel_state)
  if #state_table > PIANO_ROLL_WIDTH then
    table.remove(state_table, 1)
  end
end

local old_dmc_level = 0

local dmc_period_table = {}
dmc_period_table[428] = 0
dmc_period_table[380] = 1
dmc_period_table[340] = 2
dmc_period_table[320] = 3
dmc_period_table[286] = 4
dmc_period_table[254] = 5
dmc_period_table[226] = 6
dmc_period_table[214] = 7
dmc_period_table[190] = 8
dmc_period_table[160] = 9
dmc_period_table[142] = 10
dmc_period_table[128] = 11
dmc_period_table[106] = 12
dmc_period_table[84]  = 13
dmc_period_table[72]  = 14
dmc_period_table[54]  = 15

function update_dmc_roll(channel, state_table)
  local channel_state = {}
  local dmc_playing = channel.bytesRemaining > 0
  local delta = math.abs(channel.outputVolume - old_dmc_level)
  channel_state.playing = dmc_playing
  channel_state.delta = delta
  channel_state.address = channel.sampleAddr
  
  if dmc_period_table[channel.period + 1] then
    channel_state.rate = dmc_period_table[channel.period + 1]
  end
  table.insert(state_table, channel_state)
  if #state_table > PIANO_ROLL_WIDTH then
    table.remove(state_table, 1)
  end
  old_dmc_level = channel.outputVolume
end

function draw_piano_roll(emu, state_table, duty_colors)  
  for x = 1, #state_table do
    if state_table[x].enabled and state_table[x].y then
      local volume = state_table[x].volume
      local y = state_table[x].y - 2
      local foreground = duty_colors[state_table[x].duty + 1]
      -- outer lines, drawn at brightness dependant on volume
      local outermost_volume = math.max(0, volume - 8) / 8
      local outermost_brightness = math.min(1.0, outermost_volume)
      emu.drawLine(x - 1, y-2, x - 1, y+2, apply_transparency(foreground, outermost_brightness))
      local innermost_volume = volume / 8
      local innermost_brightness = math.min(1.0, innermost_volume)
      emu.drawLine(x - 1, y-1, x - 1, y+1, apply_transparency(foreground, innermost_brightness))
      -- center line, always drawn at full brightness when playing
      emu.drawPixel(x - 1, y, foreground)
    end
  end
end

function draw_noise_roll(emu, state_table, duty_colors)  
  for x = 1, #state_table do
    if state_table[x].enabled and state_table[x].y then
      local volume = state_table[x].volume
      local y = state_table[x].y
      local foreground = duty_colors[1]
      if state_table[x].mode then
        foreground = duty_colors[2]
      end
      -- outer lines, drawn at brightness dependant on volume
      local outermost_volume = math.max(0, volume - 8) / 8
      local outermost_brightness = math.min(1.0, outermost_volume)
      emu.drawLine(x - 1, NOISE_ROLL_OFFSET + y-2, x - 1, NOISE_ROLL_OFFSET + y+2, apply_transparency(foreground, outermost_brightness))
      local innermost_volume = volume / 8
      local innermost_brightness = math.min(1.0, innermost_volume)
      emu.drawLine(x - 1, NOISE_ROLL_OFFSET + y-1, x - 1, NOISE_ROLL_OFFSET + y+1, apply_transparency(foreground, innermost_brightness))
      -- center line, always drawn at full brightness when playing
      emu.drawPixel(x - 1, NOISE_ROLL_OFFSET + y, foreground)
    end
  end
end

function draw_piano_background()
   emu.drawRectangle(0, 0, 256, 240, BACKGROUND_COLOR, true)
end

function draw_piano_strings()
  local black_string = PIANO_STRING_BLACK_COLOR
  local white_string = PIANO_STRING_WHITE_COLOR
  local string_colors = {
    white_string, --C
    white_string, --B
    black_string, --Bb
    white_string, --A
    black_string, --Ab
    white_string, --G
    black_string, --Gb
    white_string, --F
    white_string, --E
    black_string, --Eb
    white_string, --D
    black_string, --Db
  }
  
  for i = 0, PIANO_ROLL_KEYS do
    local string_color = string_colors[i%12 + 1]
    local y = i*PIANO_ROLL_KEY_HEIGHT
    emu.drawLine(0, y, PIANO_ROLL_WIDTH, y, string_color)
  end
end

function draw_noise_strings()
  local noise_string_colors = {
    NOISE_STRING_BLACK_COLOR,
    NOISE_STRING_WHITE_COLOR
  }
  for i = 0, 15 do
    local y = i * NOISE_KEY_HEIGHT
    emu.drawLine(0, NOISE_ROLL_OFFSET + y, PIANO_ROLL_WIDTH, NOISE_ROLL_OFFSET + y, noise_string_colors[(i%2)+1])
  end
end

local white_key_border = 0x404040
local white_key = 0x505050
local black_key = 0x000000
local black_key_border = 0x181818
local upper_key_pixels = {
  white_key, -- C
  white_key_border, 
  white_key, -- B
  black_key, black_key, black_key, -- Bb
  white_key, -- A
  black_key, black_key, black_key, -- Ab
  white_key, -- G
  black_key, black_key, black_key, -- Gb
  white_key, -- F
  white_key_border,
  white_key, -- E
  black_key, black_key, black_key, -- Eb
  white_key, -- D
  black_key, black_key, black_key, -- Db
}
  
local lower_key_pixels = {
  white_key, -- C (bottom half)
  white_key_border,
  white_key, white_key, -- B
  white_key_border, 
  white_key, white_key, white_key, -- A
  white_key_border, 
  white_key, white_key, white_key, -- G
  white_key_border,
  white_key, white_key, -- F
  white_key_border,
  white_key, white_key, -- E
  white_key_border, 
  white_key, white_key, white_key, -- D
  white_key_border,
  white_key, -- C (top half)
}

function draw_piano_keys()
  -- first, draw the keys themselves
  for y = 0, PIANO_ROLL_KEYS * PIANO_ROLL_KEY_HEIGHT do
    local upper_key_color = upper_key_pixels[y % #upper_key_pixels + 1]
    local lower_key_color = lower_key_pixels[y % #lower_key_pixels + 1]
    emu.drawLine(240, y, 248, y, upper_key_color)
    emu.drawLine(248, y, 256, y, lower_key_color)
  end
  
  -- now clean up the border between the key and the piano roll
  emu.drawLine(240, 0, 240, PIANO_ROLL_HEIGHT, black_key_border)
end

function draw_right_white_key(y, color)  
  emu.drawLine(248, y + 1, 256, y + 1, color)
  emu.drawLine(240, y, 256, y, color)
end

function draw_center_white_key(y, color)
  emu.drawLine(248, y - 1, 256, y - 1, color)
  emu.drawLine(240, y, 256, y, color)
  emu.drawLine(248, y + 1, 256, y + 1, color)
end

function draw_left_white_key(y, color)
  emu.drawLine(240, y, 256, y, color)
  emu.drawLine(248, y - 1, 256, y - 1, color)
end

function draw_black_key(y, color)
  emu.drawLine(241, y - 1, 247, y - 1, color)
  emu.drawLine(241, y, 247, y, color)
  emu.drawLine(241, y + 1, 247, y + 1, color)
end

function draw_key_spot(note, duty_colors)
  if (note.y == nil or note.enabled == false) then
    return
  end
  local key_drawing_functions = {
    draw_left_white_key,  --C
    draw_right_white_key, --B
    draw_black_key,       --Bb
    draw_center_white_key,--A
    draw_black_key,       --Ab
    draw_center_white_key,--G
    draw_black_key,       --Gb
    draw_left_white_key,  --F
    draw_right_white_key, --E
    draw_black_key,       --Eb
    draw_center_white_key,--D
    draw_black_key,       --Db
  }

  local base_color = duty_colors[note.duty + 1]

  -- Calculate two key spots, based on tuning. This helps to
  -- highlight intentionally out of tune notes, and lights up the closest
  -- piano keys relative to standard tuning. The brightness of each pad is
  -- relative to the tuning offset, so the brighter key is (usually) the
  -- real note
  
  local note_key = (note.y - 2) / PIANO_ROLL_KEY_HEIGHT
  local base_key = math.floor(note_key)
  local base_percent = 1.0 - (note_key % 1)
  local adjacent_key = math.ceil(note_key)
  local adjacent_percent = note_key % 1

  -- Now take both of these colors and apply some transparency based on volume
  local volume_percent = (note.volume / 15) * 0.75 + 0.25
  base_percent = base_percent * volume_percent
  adjacent_percent = adjacent_percent * volume_percent

  local base_y = base_key * PIANO_ROLL_KEY_HEIGHT
  local base_key_color = apply_transparency(base_color, base_percent)
  
  local adjacent_y = adjacent_key * PIANO_ROLL_KEY_HEIGHT
  local adjacent_key_color = apply_transparency(base_color, adjacent_percent)

  key_drawing_functions[base_key % 12 + 1](base_y, base_key_color)
  key_drawing_functions[adjacent_key % 12 + 1](adjacent_y, adjacent_key_color)
end

function draw_pad(x, y, value, foreground, background)
  emu.drawRectangle(x, y-1, 4, 9, background, true)
  emu.drawLine(x+4, y, x+4, y+6, background)
  tiny_hex_char(x+1,y+1,value,foreground)
end

function draw_noise_pads(active_note, duty_colors)
  local pad_colors = {
    0x101010,
    0x121212,
    0x141414,
    0x161616,
    0x181818,
    0x1A1A1A,
    0x1C1C1C,
    0x1E1E1E,
    0x202020,
    0x222222,
    0x242424,
    0x262626,
    0x282828,
    0x2A2A2A,
    0x2C2C2C,
    0x2E2E2E,
  }

  for i = 0, 15 do
    local x = 240 + ((i % 4) * 4) 
    local y = i * 2 + NOISE_ROLL_OFFSET - 3
    local color = pad_colors[(0xF - i) + 1]
    local hex_value = 0xF - i;
    if (active_note.y and active_note.enabled and active_note.period == i) then
      color = duty_colors[1]
      if active_note.mode then
        color = duty_colors[2]
      end
    end
    draw_pad(x, y, hex_value, apply_brightness(color, 1.8), color)
  end

  emu.drawLine(240, NOISE_ROLL_OFFSET - 4, 240, NOISE_ROLL_OFFSET + 30, 0x101010)
  emu.drawLine(241, NOISE_ROLL_OFFSET + 30, 243, NOISE_ROLL_OFFSET + 30, NOISE_STRING_BLACK_COLOR)
end

function draw_dmc_roll(emu, state_table)
  for x = 1, #state_table do
    local y = DMC_OFFSET
    local color = DMC_PLAYING_COLOR
    if not state_table[x].playing then
      color = DMC_BASE_COLOR
    end
    -- width of "sample" based on delta, maximum is 127
    local sample_offset = math.floor((state_table[x].delta / 127) * DMC_HEIGHT)
    emu.drawLine(x - 1, y - sample_offset, x - 1, y + sample_offset, color)
  end
end

function draw_dmc_head(note)
  local foreground = 0x404040
  local background = 0x202020
  if note.playing then
    foreground = DMC_PLAYING_COLOR
    background = apply_brightness(DMC_PLAYING_COLOR, 0.5)
  end
  emu.drawRectangle(242, DMC_OFFSET - 6, 13, 13, background, true)
  emu.drawRectangle(241, DMC_OFFSET - 5, 15, 11, background, true)
  emu.drawLine(240, DMC_OFFSET - 3, 240, DMC_OFFSET + 3, 0x101010)
  tiny_hex(241, DMC_OFFSET - 2, note.address, foreground, 4)
end

local old_mouse_state = {}

function is_region_clicked(x ,y, width, height, mouse_state)
  if mouse_state.left and not old_mouse_state.left then
    if mouse_state.x >= x and mouse_state.x < x + width and mouse_state.y >= y and mouse_state.y < y + height then
      return true
    end
  end
  return false
end

function handle_input()
  local mouse_state = emu.getMouseState()
  if is_region_clicked(0, 0, 256, 240, mouse_state) then
    toggle_background()
  end
  old_mouse_state = mouse_state
end

-- drawing functions for various icons in the register display
function draw_raw_registers(x, y, box_color, shadow_color, numeral_color, background_color, values)
  emu.drawRectangle(x+1, y+1, 36, 7, shadow_color)
  emu.drawRectangle(x, y, 36, 7, box_color, true)
  for i = 1, 4 do
    emu.drawRectangle(x + ((i - 1) * 9) + 1, y + 1, 7, 5, background_color, true)
    if values[i] ~= nil then
      tiny_hex(x + ((i - 1) * 9) + 1, y + 1, values[i], numeral_color, 2)
    end
  end
end

function draw_duty_12(x, y, color)
  emu.drawLine(x, y+3, x+4, y+3, color)
  emu.drawLine(x+1, y+1, x+1, y+2, color)
end

function draw_duty_25(x, y, color)
  emu.drawLine(x, y+3, x+1, y+3, color)
  emu.drawLine(x+3, y+3, x+4, y+3, color)
  emu.drawLine(x+1, y+1, x+3, y+1, color)
  emu.drawPixel(x+1, y+2, color)
  emu.drawPixel(x+3, y+2, color)
end

function draw_duty_50(x, y, color)
  emu.drawLine(x, y+3, x+2, y+3, color)
  emu.drawLine(x+2, y+1, x+4, y+1, color)
  emu.drawPixel(x+2, y+2, color)
end

function draw_duty_75(x, y, color)
  emu.drawLine(x, y+1, x+1, y+1, color)
  emu.drawLine(x+3, y+1, x+4, y+1, color)
  emu.drawLine(x+1, y+3, x+3, y+3, color)
  emu.drawPixel(x+1, y+2, color)
  emu.drawPixel(x+3, y+2, color)
end

local duty_icon_functions = {draw_duty_12, draw_duty_25, draw_duty_50, draw_duty_75}

function draw_duty_indicator(x, y, icon_color, box_color, shadow_color, light_color, dark_color, selected_light_color, selected_dark_color, duty_cycle, playing)
  local icon_color = light_color
  if playing then
    icon_color = selected_light_color
  end
  emu.drawLine(x, y+6, x+2, y+6, icon_color)
  emu.drawLine(x+2, y, x+2, y+5, icon_color)
  emu.drawLine(x+3, y, x+6, y, icon_color)
  emu.drawLine(x+7, y, x+7, y+5, icon_color)
  emu.drawLine(x+7, y+6, x+9, y+6, icon_color)

  emu.drawRectangle(x+12, y+1, 25, 7, shadow_color)
  emu.drawRectangle(x+11, y, 25, 7, box_color, true)

  for i = 1, 4 do
    if playing and duty_cycle == (i - 1) then
      emu.drawRectangle(x + ((i - 1) * 6) + 12, y + 1, 5, 5, selected_dark_color, true)
      duty_icon_functions[i](x + ((i - 1) * 6) + 12, y + 1, selected_light_color)
    else
      emu.drawRectangle(x + ((i - 1) * 6) + 12, y + 1, 5, 5, dark_color, true)
      duty_icon_functions[i](x + ((i - 1) * 6) + 12, y + 1, light_color)
    end
  end
end

function draw_sweep_indicator(x, y, icon_color, box_color, shadow_color, light_color, dark_color, selected_light_color, selected_dark_color, sweep_byte)
  local negate = ((sweep_byte & 0x40) ~= 0)
  local period = ((sweep_byte & 0x70) >> 4)
  local shift = ((sweep_byte & 0x07))
  local enabled = ((sweep_byte & 0x80) ~= 0) and (shift ~= 0)

  local icon_color = light_color
  if enabled then
    icon_color = selected_light_color
  end

  -- sweep icon
  emu.drawLine(x, y, x+6, y+6, icon_color)
  emu.drawLine(x+3, y+4, x+5, y+6, icon_color)
  emu.drawLine(x+3, y+5, x+4, y+6, icon_color)
  emu.drawLine(x+4, y+3, x+6, y+5, icon_color)
  emu.drawLine(x+7, y+4, x+7, y+5, icon_color)
  emu.drawPixel(x+9, y+4, icon_color)
  emu.drawPixel(x+8, y+2, icon_color)

  -- bar graph indicators
  emu.drawPixel(x+21, y, icon_color)
  emu.drawPixel(x+22, y+1, icon_color)
  emu.drawPixel(x+21, y+2, icon_color)
  emu.drawPixel(x+24, y, icon_color)
  emu.drawPixel(x+25, y+1, icon_color)
  emu.drawPixel(x+24, y+2, icon_color)

  emu.drawPixel(x+21, y+5, icon_color)
  emu.drawLine(x+22, y+4, x+22, y+6, icon_color)
  emu.drawLine(x+24, y+4, x+24, y+6, icon_color)
  emu.drawPixel(x+25, y+5, icon_color)

  -- indicator outlines
  emu.drawRectangle(x+12, y+1, 7, 7, shadow_color)
  emu.drawRectangle(x+11, y, 7, 7, box_color, true)

  emu.drawRectangle(x+28, y+1, 9, 7, shadow_color)
  emu.drawRectangle(x+27, y, 9, 7, box_color, true)

  if enabled then
    emu.drawRectangle(x+12, y+1, 5, 5, selected_dark_color, true)
    emu.drawLine(x+13, y+3, x+15, y+3, selected_light_color)
    if negate then
      emu.drawPixel(x+14, y+2, selected_light_color)
      emu.drawLine(x+12, y+4, x+16, y+4, selected_light_color)
    else
      emu.drawPixel(x+14, y+4, selected_light_color)
      emu.drawLine(x+12, y+2, x+16, y+2, selected_light_color)
    end
    emu.drawRectangle(x+28, y+1, 7, 2, selected_dark_color)
    emu.drawRectangle(x+28, y+4, 7, 2, selected_dark_color)
    emu.drawRectangle(x+28, y+1, shift, 2, selected_light_color)
    emu.drawRectangle(x+28, y+4, period, 2, selected_light_color)
  else
    -- simply blank out the indicators and draw nothing
    emu.drawRectangle(x+12, y+1, 5, 5, dark_color, true)
    emu.drawRectangle(x+28, y+1, 7, 2, dark_color)
    emu.drawRectangle(x+28, y+4, 7, 2, dark_color)
  end
end

function draw_volume_envelope_indicator(x, y, icon_color, box_color, shadow_color, light_color, dark_color, selected_light_color, selected_dark_color, volume_byte)
  local constant_volume = ((volume_byte & 0x10) ~= 0)

  local icon_color = light_color
  if not constant_volume then
    icon_color = selected_light_color
  end

  -- volume icon
  emu.drawLine(x, y+2, x, y+4, icon_color)
  emu.drawPixel(x+1, y+2, icon_color)
  emu.drawPixel(x+1, y+4, icon_color)
  emu.drawLine(x+2, y+1, x+2, y+5, icon_color)
  emu.drawLine(x+3, y, x+3, y+1, icon_color)
  emu.drawLine(x+3, y+5, x+3, y+6, icon_color)
  emu.drawPixel(x+4, y, icon_color)
  emu.drawPixel(x+4, y+6, icon_color)
  emu.drawLine(x+5, y, x+5, y+6, icon_color)
  emu.drawLine(x+7, y+2, x+7, y+4, icon_color)
  emu.drawLine(x+7, y, x+8, y, icon_color)
  emu.drawLine(x+7, y+6, x+8, y+6, icon_color)
  emu.drawLine(x+9, y+1, x+9, y+5, icon_color)

  -- box backgrounds and scaffolding
  emu.drawRectangle(x+12, y+1, 13, 7, shadow_color)
  emu.drawRectangle(x+11, y, 13, 7, box_color, true)

  emu.drawLine(x+25, y+4, x+32, y+4, shadow_color)
  emu.drawLine(x+24, y+3, x+31, y+3, box_color)

  emu.drawRectangle(x+32, y+1, 5, 7, shadow_color)
  emu.drawRectangle(x+31, y, 5, 7, box_color, true)

  if constant_volume then
    -- draw un-highlighted icons
    emu.drawRectangle(x+12, y+1, 5, 5, dark_color, true)
    emu.drawRectangle(x+12, y+1, 2, 5, light_color, true)
    emu.drawRectangle(x+14, y+2, 2, 3, light_color, true)
    emu.drawPixel(x+16, y+3, light_color)

    emu.drawRectangle(x+18, y+1, 5, 5, dark_color, true)
    emu.drawPixel(x+19, y+2, light_color)
    emu.drawPixel(x+19, y+4, light_color)
    emu.drawLine(x+21, y+1, x+21, y+5, light_color)

    -- do not draw envelope period, but blank the cell
    emu.drawRectangle(x+32, y+1, 3, 5, dark_color, true)
  else
    -- draw uhighlighted icons
    emu.drawRectangle(x+12, y+1, 5, 5, selected_dark_color, true)
    emu.drawRectangle(x+12, y+1, 2, 5, selected_light_color, true)
    emu.drawRectangle(x+14, y+2, 2, 3, selected_light_color, true)
    emu.drawPixel(x+16, y+3, selected_light_color) 

    local loop = (volume_byte & 0x30) ~= 0
    if loop then
      emu.drawRectangle(x+18, y+1, 5, 5, selected_dark_color, true)
      emu.drawPixel(x+19, y+2, selected_light_color)
      emu.drawPixel(x+19, y+4, selected_light_color)
      emu.drawLine(x+21, y+1, x+21, y+5, selected_light_color)
    else
      emu.drawRectangle(x+18, y+1, 5, 5, dark_color, true)
      emu.drawPixel(x+19, y+2, light_color)
      emu.drawPixel(x+19, y+4, light_color)
      emu.drawLine(x+21, y+1, x+21, y+5, light_color)
    end

    -- populate the envelope period
    local period = (volume_byte & 0xF)
    emu.drawRectangle(x+32, y+1, 3, 5, selected_dark_color, true)
    tiny_hex(x+32, y+1, period, selected_light_color, 1)
  end
end

function draw_volume_bar(x, y, icon_color, box_color, shadow_color, light_color, dark_color, selected_light_color, selected_dark_color, volume_byte, current_volume)
  emu.drawRectangle(x+1, y+1, 36, 7, shadow_color)
  emu.drawRectangle(x, y, 36, 7, box_color, true)

  -- first, handle the constant volume indicator
  local constant_volume = ((volume_byte & 0x10) ~= 0)
  if constant_volume then
    emu.drawRectangle(x+1, y+1, 3, 5, selected_dark_color, true)
    tiny_hex(x+1, y+1, current_volume, selected_light_color, 1)
  else
    emu.drawRectangle(x+1, y+1, 3, 5, dark_color, true)
  end

  -- then, based on the current playing volume, update the bar. Note this is the output volume, NOT the
  -- value written to the register. It should follow the active envelope.
  for i = 1, 15 do
    local dx = x+5 + ((i-1) * 2)
    local dy = y+1
    if current_volume >= i then
      emu.drawLine(dx, dy, dx, dy+4, selected_dark_color)
      emu.drawLine(dx+1, dy, dx+1, dy+4, selected_light_color)
    else
      emu.drawLine(dx, dy, dx, dy+4, dark_color)
      emu.drawLine(dx+1, dy, dx+1, dy+4, light_color)
    end
  end
end

function draw_triangle_indicator(x, y, icon_color, box_color, shadow_color, light_color, dark_color, selected_light_color, selected_dark_color, triangle_playing)
  local icon_color = light_color
  if triangle_playing then
    icon_color = selected_light_color
  end

  emu.drawLine(x, y+5, x+4, y+1, icon_color)
  emu.drawLine(x+5, y+1, x+9, y+5, icon_color)

  emu.drawRectangle(x+12, y+1, 25, 7, shadow_color)
  emu.drawRectangle(x+11, y, 25, 7, box_color, true)

  if triangle_playing then
    emu.drawRectangle(x+12, y+1, 23, 5, selected_dark_color, true)
    emu.drawRectangle(x+21, y+1, 2, 5, selected_light_color)
    emu.drawRectangle(x+23, y+2, 2, 3, selected_light_color)
    emu.drawLine(x+25, y+3, x+26, y+3, selected_light_color)
  else
    emu.drawRectangle(x+12, y+1, 23, 5, dark_color, true) 
    emu.drawRectangle(x+21, y+1, 2, 5, light_color)
    emu.drawRectangle(x+23, y+2, 2, 3, light_color)
    emu.drawLine(x+25, y+3, x+26, y+3, light_color)
  end
end

function draw_mode_0(x, y, color)
  emu.drawPixel(x, y+2, color)
  emu.drawPixel(x+1, y, color)
  emu.drawPixel(x+2, y+3, color)
  emu.drawPixel(x+3, y+4, color)
  emu.drawPixel(x+4, y+2, color)
  emu.drawPixel(x+5, y, color)
  emu.drawPixel(x+6, y+2, color)
  emu.drawPixel(x+7, y+3, color)
  emu.drawPixel(x+8, y+1, color)
  emu.drawPixel(x+9, y+4, color)
  emu.drawPixel(x+10, y, color)
end

function draw_mode_1(x, y, color)
  emu.drawPixel(x, y+2, color)
  emu.drawLine(x+1, y+1, x+1, y+2, color)
  emu.drawPixel(x+2, y, color)
  emu.drawPixel(x+3, y+4, color)
  emu.drawPixel(x+4, y+3, color)
  emu.drawLine(x+5, y+2, x+5, y+3, color)
  emu.drawPixel(x+6, y+1, color)
  emu.drawPixel(x+7, y, color)
  emu.drawLine(x+8, y+3, x+8, y+4, color)
  emu.drawPixel(x+9, y+2, color)
  emu.drawPixel(x+10, y+1, color)
end

local mode_functions = {}
mode_functions[0] = draw_mode_0
mode_functions[1] = draw_mode_1

function draw_noise_mode(x, y, icon_color, box_color, shadow_color, light_color, dark_color, selected_light_color, selected_dark_color, noise_byte, playing)
  local icon_color = light_color
  if playing then
    icon_color = selected_light_color
  end

  emu.drawPixel(x, y+3, icon_color)
  emu.drawPixel(x+1, y+2, icon_color)
  emu.drawLine(x+2, y+3, x+2, y+5, icon_color)
  emu.drawLine(x+3, y, x+3, y+2, icon_color)
  emu.drawLine(x+4, y+3, x+4, y+4, icon_color)
  emu.drawPixel(x+5, y+2, icon_color)
  emu.drawLine(x+6, y+3, x+6, y+6, icon_color)
  emu.drawLine(x+7, y+1, x+7, y+2, icon_color)
  emu.drawLine(x+8, y+3, x+9, y+3, icon_color)

  emu.drawRectangle(x+12, y+1, 25, 7, shadow_color)
  emu.drawRectangle(x+11, y, 25, 7, box_color, true)

  local noise_mode = ((noise_byte & 0x80) >> 7)
  for i = 0, 1 do
    if i == noise_mode then
      emu.drawRectangle(x+12 + (i*12), y+1, 11, 5, selected_dark_color, true)
      mode_functions[i](x+12 + (i*12), y+1, selected_light_color)
    else
      emu.drawRectangle(x+12 + (i*12), y+1, 11, 5, dark_color, true)
      mode_functions[i](x+12 + (i*12), y+1, light_color)
    end
  end
end

function draw_dpcm_sample_indicator(x, y, icon_color, box_color, shadow_color, light_color, dark_color, selected_light_color, selected_dark_color, frequency_byte, sample_address_byte, dpcm_active)
  local icon_color = light_color
  if dpcm_active then
    icon_color = selected_light_color
  end

  emu.drawLine(x, y+3, x+9, y+3, icon_color)
  emu.drawLine(x+1, y+2, x+1, y+4, icon_color)
  emu.drawLine(x+2, y+1, x+2, y+5, icon_color)
  emu.drawLine(x+4, y+2, x+4, y+4, icon_color)
  emu.drawLine(x+6, y+1, x+6, y+5, icon_color)
  emu.drawLine(x+7, y, x+7, y+6, icon_color)
  emu.drawLine(x+8, y+2, x+8, y+4, icon_color)

  emu.drawRectangle(x+13, y+1, 5, 7, shadow_color)
  emu.drawRectangle(x+12, y, 5, 7, box_color, true)

  emu.drawLine(x+18, y+4, x+19, y+4, shadow_color)
  emu.drawLine(x+17, y+3, x+18, y+3, box_color)

  emu.drawRectangle(x+20, y+1, 17, 7, shadow_color)
  emu.drawRectangle(x+19, y, 17, 7, box_color, true)

  local frequency = frequency_byte & 0xF
  local address = 0xC000 + (sample_address_byte << 6)

  if dpcm_active then
    emu.drawRectangle(x+13, y+1, 3, 5, selected_dark_color, true)
    emu.drawRectangle(x+20, y+1, 15, 5, selected_dark_color, true)
    tiny_hex(x+13, y+1, frequency, selected_light_color, 1)
    tiny_hex(x+20, y+1, address, selected_light_color, 4)
  else
    emu.drawRectangle(x+13, y+1, 3, 5, dark_color, true)
    emu.drawRectangle(x+20, y+1, 15, 5, dark_color, true)
    tiny_hex(x+13, y+1, frequency, light_color, 1)
    tiny_hex(x+20, y+1, address, light_color, 4)
  end
end

function draw_dpcm_length_indicator(x, y, icon_color, box_color, shadow_color, light_color, dark_color, selected_light_color, selected_dark_color, control_byte, length_byte, dpcm_active)
  local icon_color = light_color
  if dpcm_active then
    icon_color = selected_light_color
  end

  emu.drawLine(x+3, y, x+5, y, icon_color)
  emu.drawLine(x+7, y+2, x+7, y+4, icon_color)
  emu.drawLine(x+3, y+6, x+5, y+6, icon_color)
  emu.drawLine(x+1, y+2, x+1, y+4, icon_color)
  emu.drawPixel(x+2,y+1, icon_color)
  emu.drawPixel(x+6,y+1, icon_color)
  emu.drawPixel(x+2,y+5, icon_color)
  emu.drawPixel(x+6,y+5, icon_color)
  emu.drawLine(x+4, y+1, x+4, y+3, icon_color)
  emu.drawLine(x+5, y+3, x+6, y+3, icon_color)

  emu.drawRectangle(x+18, y+1, 19, 7, shadow_color)
  emu.drawRectangle(x+17, y, 19, 7, box_color, true)

  local looping = ((control_byte & 0x40) ~= 0)
  local sample_length = (length_byte * 16) + 1

  if dpcm_active then
    emu.drawRectangle(x+18,y+1,11, 5, selected_dark_color, true)
    emu.drawRectangle(x+30,y+1,5, 5, selected_dark_color, true)
    tiny_hex(x+18,y+1,sample_length, selected_light_color, 3)
    emu.drawPixel(x+31, y+2, selected_light_color)
    emu.drawPixel(x+31, y+4, selected_light_color)
    emu.drawLine(x+33, y+1, x+33, y+5, selected_light_color)
  else
    emu.drawRectangle(x+18,y+1,11, 5, dark_color, true)
    emu.drawRectangle(x+30,y+1,5, 5, dark_color, true)
    tiny_hex(x+18,y+1,sample_length, light_color, 3)
    emu.drawPixel(x+31, y+2, light_color)
    emu.drawPixel(x+31, y+4, light_color)
    emu.drawLine(x+33, y+1, x+33, y+5, light_color)
  end
end

local ICON_COLOR = 0x808080
local SHADOW_COLOR = 0x80000000
local BOX_OUTLINE_COLOR = 0x000000
local UNSELECTED_DARK_COLOR = 0x404040
local UNSELECTED_LIGHT_COLOR = 0x808080

function draw_apu_registers()
  emu.drawRectangle(0, 0, 39, 240, 0x40202020, true)

  local square1_duty = (shadow_apu[0x4000] & 0xC0) >> 6
  local square1_selected_light = SQUARE1_COLORS[square1_duty + 1]
  local square1_selected_dark = square1_selected_light + 0xB0000000

  tiny_string(1, 1, "Pulse 1", 0xFFFFFF)
  draw_raw_registers(1, 7, BOX_OUTLINE_COLOR, SHADOW_COLOR, UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR,
    {shadow_apu[0x4000],shadow_apu[0x4001],shadow_apu[0x4002],shadow_apu[0x4003]})
  draw_duty_indicator(1, 16, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    square1_selected_light, square1_selected_dark, -- icon color when highlighted
    (shadow_apu[0x4000] & 0xC0) >> 6, square1_roll[#square1_roll].enabled)

  draw_sweep_indicator(1, 25, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    square1_selected_light, square1_selected_dark, -- icon color when highlighted
    shadow_apu[0x4001])

  draw_volume_envelope_indicator(1, 34, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    square1_selected_light, square1_selected_dark, -- icon color when highlighted
    shadow_apu[0x4000])

  draw_volume_bar(1, 43, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    square1_selected_light, square1_selected_dark, -- icon color when highlighted
    shadow_apu[0x4000],
    square1_roll[#square1_roll].volume)

  local square2_duty = (shadow_apu[0x4004] & 0xC0) >> 6
  local square2_selected_light = SQUARE2_COLORS[square2_duty + 1]
  local square2_selected_dark = square2_selected_light + 0xB0000000

  tiny_string(1, 60, "Pulse 2", 0xFFFFFF)
  draw_raw_registers(1, 66, BOX_OUTLINE_COLOR, SHADOW_COLOR, UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR,
    {shadow_apu[0x4004],shadow_apu[0x4005],shadow_apu[0x4006],shadow_apu[0x4007]})
  draw_duty_indicator(1, 75, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    square2_selected_light, square2_selected_dark, -- icon color when highlighted
    (shadow_apu[0x4004] & 0xC0) >> 6, square2_roll[#square2_roll].enabled)

  draw_sweep_indicator(1, 84, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    square2_selected_light, square2_selected_dark, -- icon color when highlighted
    shadow_apu[0x4005])

  draw_volume_envelope_indicator(1, 93, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    square2_selected_light, square2_selected_dark, -- icon color when highlighted
    shadow_apu[0x4004])

  draw_volume_bar(1, 102, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    square2_selected_light, square2_selected_dark, -- icon color when highlighted
    shadow_apu[0x4004],
    square2_roll[#square2_roll].volume)

  local triangle_selected_light = TRIANGLE_COLORS[1]
  local triangle_selected_dark = triangle_selected_light + 0xB0000000

  tiny_string(1, 119, "Triangle", 0xFFFFFF)

  draw_raw_registers(1, 126, BOX_OUTLINE_COLOR, SHADOW_COLOR, UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR,
    {shadow_apu[0x4008],nil,shadow_apu[0x400A],shadow_apu[0x400B]})

  draw_triangle_indicator(1, 135, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    triangle_selected_light, triangle_selected_dark, -- icon color when highlighted
    triangle_roll[#triangle_roll].enabled)

  local noise_mode = ((shadow_apu[0x400E] & 0x80) >> 7)
  local noise_selected_light = NOISE_COLORS[noise_mode+1]
  local noise_selected_dark = noise_selected_light + 0xB0000000

  tiny_string(1, 195, "Noise", 0xFFFFFF)

  draw_raw_registers(1, 202, BOX_OUTLINE_COLOR, SHADOW_COLOR, UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR,
    {shadow_apu[0x400C],nil,shadow_apu[0x400E],shadow_apu[0x400F]})
  draw_noise_mode(1, 211, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    noise_selected_light, noise_selected_dark, -- icon color when highlighted
    shadow_apu[0x400E], noise_roll[#noise_roll].enabled)


  draw_volume_envelope_indicator(1, 220, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    noise_selected_light, noise_selected_dark, -- icon color when highlighted
    shadow_apu[0x400C])

  draw_volume_bar(1, 229, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    noise_selected_light, noise_selected_dark, -- icon color when highlighted
    shadow_apu[0x400C],
    noise_roll[#noise_roll].volume)

  local dpcm_selected_light = DMC_PLAYING_COLOR
  local dpcm_selected_dark = dpcm_selected_light + 0xB0000000

  tiny_string(1, 159, "DPCM", 0xFFFFFF)

  draw_raw_registers(1, 166, BOX_OUTLINE_COLOR, SHADOW_COLOR, UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR,
    {shadow_apu[0x4010],shadow_apu[0x4011],shadow_apu[0x4012],shadow_apu[0x4013]})

  draw_dpcm_sample_indicator(1, 175, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    dpcm_selected_light, dpcm_selected_dark, -- icon color when highlighted
    shadow_apu[0x4010], shadow_apu[0x4012], dmc_roll[#dmc_roll].playing)

  draw_dpcm_length_indicator(1, 184, 
    ICON_COLOR, --line color
    BOX_OUTLINE_COLOR, SHADOW_COLOR, -- box outline and shadow
    UNSELECTED_LIGHT_COLOR, UNSELECTED_DARK_COLOR, -- icon color when darkened
    dpcm_selected_light, dpcm_selected_dark, -- icon color when highlighted
    shadow_apu[0x4010], shadow_apu[0x4013], dmc_roll[#dmc_roll].playing)
end

function mesen_draw()
  local state = emu.getState()
  local clockRate = state["clockRate"]
  --             x   y   str            fore: argb  back: argb 
  --emu.drawString(10, 10, "Hello Mesen",   0x80ff0000, 0x80FFFFFF)
  --emu.drawString(10, 20, "P1: "..apu.square1.period,   0xff0000, 0xFFFFFF)

  local apu = {
    square1 = {
      frequency = clockRate / 16 / (state["apu.square1.realPeriod"] + 1),
      envelope = { volume = state["apu.square1.envelope.volume"] },
      lengthCounter = { counter = state["apu.square1.envelope.lengthCounter.counter"] },
      duty = state["apu.square1.duty"]
     },
     square2 = {
      frequency = clockRate / 16 / (state["apu.square2.realPeriod"] + 1),
      envelope = { volume = state["apu.square2.envelope.volume"] },
      lengthCounter = { counter = state["apu.square2.envelope.lengthCounter.counter"] },
      duty = state["apu.square2.duty"]
     },
     triangle = {
       frequency = clockRate / 32 / (state["apu.triangle.timer.period"] + 1),
       lengthCounter = { counter = state["apu.triangle.lengthCounter.counter"] },
       period = state["apu.triangle.timer.period"],
       enabled = state["apu.triangle.lengthCounter.enabled"]
     },
     noise = {
       period = state["apu.noise.timer.period"],
       mode = state["apu.noise.modeFlag"],
       envelope = { 
         constantVolume = state["apu.noise.envelope.constantVolume"],
         volume = state["apu.noise.envelope.volume"],
         counter = state["apu.noise.envelope.counter"]
       },
       lengthCounter = { counter =  state["apu.noise.envelope.lengthCounter.counter"] }
     },
     dmc = {
       bytesRemaining = state["apu.dmc.bytesRemaining"],
       outputVolume = state["apu.dmc.timer.lastOutput"],
       period = state["apu.dmc.timer.period"],
       sampleAddr = state["apu.dmc.sampleAddr"],
     }
  }

  update_piano_roll(apu.square1, square1_roll)
  update_piano_roll(apu.square2, square2_roll)
  update_piano_roll(apu.triangle, triangle_roll)
  update_noise_roll(apu.noise, noise_roll)
  update_dmc_roll(apu.dmc, dmc_roll)

  draw_piano_background()
  draw_piano_strings()
  draw_piano_keys()
  draw_noise_strings()
  draw_noise_pads(noise_roll[#noise_roll], NOISE_COLORS)
  
  draw_piano_roll(emu, square1_roll, SQUARE1_COLORS) 
  draw_piano_roll(emu, square2_roll, SQUARE2_COLORS) 
  draw_piano_roll(emu, triangle_roll, TRIANGLE_COLORS)
  draw_noise_roll(emu, noise_roll, NOISE_COLORS)
  draw_dmc_roll(emu, dmc_roll)
  draw_key_spot(square1_roll[#square1_roll], SQUARE1_COLORS)
  draw_key_spot(square2_roll[#square2_roll], SQUARE2_COLORS)
  draw_key_spot(triangle_roll[#square1_roll], TRIANGLE_COLORS)
  draw_dmc_head(dmc_roll[#dmc_roll])

  draw_apu_registers()

  handle_input()
end

emu.addEventCallback(mesen_draw, emu.eventType.endFrame);