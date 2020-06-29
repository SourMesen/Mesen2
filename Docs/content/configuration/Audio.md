---
title: Audio Options
weight: 2
chapter: false
---

### General Options ###

<div class="imgBox"><div>
	<img src="/images/AudioOptions_General.png" />
	<span>General Options</span>
</div></div>

**Audio Device**: Selects which device is used for audio output (e.g computer speakers, or a headset)

**Sample Rate**: Selects the sample rate for the audio output -- typically, computers output at 44,100Hz or 48,000Hz, so they usually offer the best sound quality.

**Latency**: This represents the length of the buffer used in audio processing. A smaller value results in less delay between the audio and video, however, depending on the hardware used, a value that is too small may cause sound problems.

The **equalizer** can be used to alter the relative strength of specific frequencies.

<div class="clear"></div>

### Advanced Options ###

<div class="imgBox"><div>
	<img src="/images/AudioOptions_Advanced.png" />
	<span>Advanced Options</span>
</div></div>

* **Enable cubic interpolation**: This option replaces the SNES' default gaussian interpolation filter with a cubic interpolation filter which can produce better audio.

* **Disable dynamic sample rate**: While a game is running, the video and audio typically slowly drift out of sync.  Mesen-S will automatically make adjustments to the audio sample rate while the game is running to keep them in sync. Disabling this option will typically cause sound issues such as crackling.
