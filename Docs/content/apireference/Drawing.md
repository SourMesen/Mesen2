---
title: Drawing
weight: 10
pre: ""
chapter: false
---

## Drawing basics ##

All drawing-related functions share a few properties:  
- (x, y) coordinates must be between (0, 0) and (255, 238)  
- The "duration" is specified as a number of frames during which the drawing must remain on the screen. This defaults to 1 frame when unspecified, and draw calls will be permanent (until a call to [clearScreen](#clearscreen)) if duration is set to 0.   
- Colors are integers in ARGB format:

    White: 0xFFFFFF
	Black: 0x000000
	Red: 0xFF0000
	Green: 0x00FF00
	Blue: 0x0000FF
	
	The alpha component (transparency) can be used and defaults to being fully opaque when set to 0 or omitted. (0xFF is fully transparent)
	e.g: 
      semi-transparent black: 0x7F000000
      opaque gray: 0xFF888888 (fully transparent gray color) 

## drawPixel ##

**Syntax**  

    emu.drawPixel(x, y, color [, duration, delay])

**Parameters**  
x - *Integer* X position  
y - *Integer* Y position    
color - *Integer* Color  
duration - *Integer* Number of frames to display (Default: 1 frame)  
delay - *Integer* Number of frames to wait before drawing the pixel (Default: 0 frames)

**Return value**  
*None*

**Description**  
Draws a pixel at the specified (x, y) coordinates using the specified color for a specific number of frames.

## drawLine ##

**Syntax**  

    emu.drawLine(x, y, x2, y2, color [, duration, delay])

**Parameters**  
x - *Integer* X position (start of line)  
y - *Integer* Y position (start of line)  
x2 - *Integer* X position (end of line)  
y2 - *Integer* Y position (end of line)  
color - *Integer* Color  
duration - *Integer* Number of frames to display (Default: 1 frame)  
delay - *Integer* Number of frames to wait before drawing the line (Default: 0 frames)

**Return value**  
*None*

**Description**  
Draws a line between (x, y) to (x2, y2) using the specified color for a specific number of frames.

## drawRectangle ##

**Syntax**  

    emu.drawRectangle(x, y, width, height, color, fill [, duration, delay])

**Parameters**  
x - *Integer* X position  
y - *Integer* Y position
width - *Integer* The rectangle's width  
height - *Integer* The rectangle's height  
color - *Integer* Color  
fill - *Boolean* Whether or not to draw an outline, or a filled rectangle.  
duration - *Integer* Number of frames to display (Default: 1 frame)  
delay - *Integer* Number of frames to wait before drawing the rectangle (Default: 0 frames)

**Return value**  
*None*

**Description**  
Draws a rectangle between (x, y) to (x+width, y+height) using the specified color for a specific number of frames.  
If *fill* is false, only the rectangle's outline will be drawn.

## drawString ##

**Syntax**  

    emu.drawString(x, y, text, textColor, backgroundColor [, duration, delay])

**Parameters**  
x - *Integer* X position  
y - *Integer* Y position  
text- *String* The text to display  
textColor - *Integer* Color to use for the text  
backgroundColor - *Integer* Color to use for the text's background color  
duration - *Integer* Number of frames to display (Default: 1 frame)  
delay - *Integer* Number of frames to wait before drawing the text (Default: 0 frames)

**Return value**  
*None*

**Description**  
Draws text at (x, y) using the specified text and colors for a specific number of frames.  


## clearScreen ##

**Syntax**  

    emu.clearScreen()

**Return value**  
*None*

**Description**  
Removes all drawings from the screen.  


## getPixel ##

**Syntax**  

    emu.getPixel(x, y)

**Parameters**  
x - *Integer* X position  
y - *Integer* Y position    

**Return value**  
*Integer* ARGB color

**Description**  
Returns the color (in ARGB format) of the PPU's output for the specified location.


## getScreenBuffer ##

**Syntax**  

    emu.getScreenBuffer()

**Return value**  
*Array* 32-bit integers in ARGB format

**Description**  
Returns an array of ARGB values for the entire screen (256px by 239px) - can be used with [emu.setScreenBuffer()](#setscreenbuffer) to alter the frame.


## setScreenBuffer ##

**Syntax**  

    emu.setScreenBuffer(screenBuffer)

**Parameters**  
screenBuffer - *Array* An array of 32-bit integers in ARGB format
	
**Return value**  
*None*

**Description**  
Replaces the current frame with the contents of the specified array.

