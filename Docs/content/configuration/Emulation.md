---
title: Emulation Options
weight: 8
chapter: false
---

## General Options ##

<div class="imgBox"><div>
	<img src="/images/EmulationSettings_General.png" />
	<span>General Options</span>
</div></div>

{{% notice tip %}}
Set any speed value below to 0 to make Mesen-S run as fast as possible.
{{% /notice %}}

**Emulation Speed**: This configures the regular speed to use when emulating. This should normally be set to `100%`.

**Fast Forward Speed**: This is the alternate speed that is used when the [Fast Forward button](/configuration/preferences.html#shortcut-keys) is held down.

**Rewind Speed**: This configures the speed at which to rewind the gameplay when the [Rewind button](/configuration/preferences.html#shortcut-keys) is held down.

**Region**: When set to `Auto`, the emulator will try to detect the game's region (NTSC or PAL) - however, this is not always possible.  When there is nothing to suggest a game is for the `PAL` region (Australia & Europe), the `NTSC` region (North America & Japan) will be used by default.

**Run Ahead**: Run ahead allows the reduction of input lag by the number of frames specified. CPU requirements increase proportionally with the number of run ahead frames specified.  

 * Run ahead is currently not compatible with movies or netplay - the movies and netplay menus will be disabled if runahead is turned on.
 * **Note for speedrunners:** Using features such as run ahead to reduce lag typically counts as cheating for the purposes of speed running.

## BS-X ##

<div class="imgBox"><div>
	<img src="/images/EmulationSettings_Bsx.png" />
	<span>BS-X Options</span>
</div></div>

This section configures the date and time that BS-X/Satellaview games use. It can either be set to the current date and time, or to a specific date and time which is used each time a BS-X title is launched.

## Advanced Options ##

<div class="imgBox"><div>
	<img src="/images/EmulationSettings_Advanced.png" />
	<span>Advanced Options</span>
</div></div>

{{% notice tip %}}
When developing software for the SNES, enabling these options can help you catch bugs. Setting the power on state for RAM to **random values** is also recommended.
{{% /notice %}}

**Default power on state for RAM**: On a console, the RAM's state at power on is undetermined and relatively random. This option lets you select Mesen-S' behavior when initializing RAM - set all bits to 0, set all bits to 1, or randomize the value of each bit.

**Randomize power-on state**: When enabled, various chips (PPU, etc.) often have a random state at power-on and need to be fully initialized before being used. This option causes their power on state to be random, forcing the program to initialize them to run properly.

**Use strict board mappings**: When enabled, this option turns on stricter board mappings for some cartridges. Currently, this only affects CX4 games.


## Overclocking ##

{{% notice warning %}}
Overclocking can cause issues in some games. The safest way to overclock is to increase the `Additional scanlines before NMI` option and leave the other options to their default values.
{{% /notice %}}

<div class="imgBox"><div>
	<img src="/images/EmulationSettings_Overclocking.png" />
	<span>Overclocking Options</span>
</div></div>

**Additional scanlines before NMI**: Increases the number of scanlines in the PPU, *before* the NMI signal is triggered at the end of the visible frame. This effectively gives more time for games to perform calculations, which can reduce slowdowns in games. **This is the preferred option for overclocking.**

**Additional scanlines after NMI**: Increases the number of scanlines in the PPU, *after* the NMI signal is triggered at the end of the visible frame. This effectively gives more time for games to perform calculations, which can reduce slowdowns in games. **This option is less compatible and should only be used if the `before NMI` variation does not work as expected.**

**Super FX Clock Speed**: Increases the clock speed used for the Super FX chip, which can reduce lag in Super FX games. This method of overclocking is more efficient for Super FX titles.