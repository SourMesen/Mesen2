---
title: Video Options
weight: 6
chapter: false
---

## General Options ##

<div class="imgBox"><div>
	<img src="/images/VideoOptions_General.png" />
	<span>General Options</span>
</div></div>

**Scale**: The scale determines the emulator window's size - use integer factors (e.g: 2x, 3x, 4x) for best results.

**Aspect Ratio**: The SNES' resolution in most games is 256x224, but it used to be displayed on CRT TVs that had a rectangular picture. To simulate a CRT TV, you can use the `Auto` option - it will switch between NTSC and PAL aspect ratios depending on the game you are playing. Using anything other than the `Default (No Stretching)` option may cause pixels to have irregular sizes. You can reduce this effect by using a combination of video filters and the bilinear filtering option.

**Enable integer FPS mode**: Under normal conditions, the NTSC SNES runs at 60.1 fps. When playing a 60hz LCD, this causes a lot of dropped frames. This option slows down the emulation by a tiny amount to produce 60 frames per second instead, which reduces the number of dropped frames.

**Enable vertical sync**: Turns on vertical sync -- can help prevent screen tearing on some hardware configurations.

**Use exclusive fullscreen mode**: Turns on exclusive fullscreen mode. This may be useful if you are experiencing screen tearing issues in regular fullscreen despite vertical sync being turned on.

**Fulscreen Resolution**: This option is shown only when exclusive fullsceen mode is enabled. It allows you to select the screen resolution that should be used when in exclusive fullscreen mode. The default resolution is the current Windows screen resolution.

**Requested Refresh Rate**: This option is shown only when exclusive fullsceen mode is enabled. It allows you to select your preferred refresh rate when running in exclusive fullscreen mode.

**Use integer scale values when entering fullscreen mode**: By default, fullscreen mode fills the entire screen. However, this can cause non-integer scaling values to be used -- for example, in 1080p resolution, the scale becomes 4.5x. Since this can cause irregularly shaped pixels, you can use this option to use the nearest integer scale value instead (e.g 4x in this example).

## Picture ##

<div class="imgBox"><div>
	<img src="/images/VideoOptions_Picture.png" />
	<span>Picture Options</span>
</div></div>

**Filter**: Allows you to select a video filter. Selecting NTSC filters will cause additional configuration options to appear below.

### Common Options ###

The `Brightness`, `Contrast`, `Hue`, `Saturation`, `Scanline` settings are common to all filters and can even be used without a filter.

**Use bilinear interpolation when scaling**: When enabled, bilinear interpolation is used when stretching (due to scale or aspect ratio). When disabled, nearest neighbor scaling is used. An easy way to get a slightly-softened screen, for example, is to use the `Prescale` filters (which use nearest neighbor scaling), use a bigger scale and enable bilinear filtering. For example, try this configuration:
```text
  Filter: Prescale 3x
  Scale: 4x
  Use bilinear interpolation when scaling: Enabled
```

**Blend high resolution modes**: Some games use the SNES' "high resolution" mode which produces a 512x224 picture.
However, this mode causes horizontal blending, which is sometimes used for pseudo-transparency effects.
Enabling this option will allow these pseudo-transparency effects to look as they were intended (but causes the entire picture to look softer/blurrier)

**Scanlines**: Simulates the scanlines on a CRT TV - the higher the value, the deeper the scanlines appear on the screen.

### NTSC Filter Options ###

The `NTSC` filter available in Mesen-S is blargg's NTSC filter - this filter is very fast, and available in various other emulators.
This filter supports a large number of options which are visible on this tab when the NTSC filter is selected.

**NTSC options**: `Artifacts`, `Bleed`, `Fringing`, `Gamma`, `Resolution`, `Sharpness`  

Feel free to experiment with the settings and choose what you feel looks best.

## Overscan ##

<div class="imgBox"><div>
	<img src="/images/VideoOptions_Overscan.png" />
	<span>Overscan Options</span>
</div></div>

The overscan settings allow you to cut out pixels on any edge of the screen. On a CRT TV, a few pixels on each side of the screen are usually hidden.
Most SNES games output 224 scanlines, while others use the SNES' 239 scanlines mode.
To avoid the window or picture size changing when the game changes between either mode, Mesen-S always outputs 239 scanlines.
In the vast majority of games, this results in 7 blank lines on the top and 8 on the bottom. To hide these blank scanlienes, set the top overscan value to 7 and the bottom to 8 (these are the default values)

## Advanced Options ##

<div class="imgBox"><div>
	<img src="/images/VideoOptions_Advanced.png" />
	<span>Advanced Options</span>
</div></div>

**Hide background layer 0-3**: The SNES can display up to 4 different background layers. These options control whether or not each layer is shown on the screen.

**Hide sprites**: When enabled, all sprites will be hidden.

**Disable frame skipping while fast forwarding**: By default, Mesen-S skips some frames when fast forwarding to improve performance. When enabled, this option forces the emulator to render every single frame, which makes fast forwarding slower.
