---
title: Trace Logger
weight: 35
chapter: false
---

<div class="imgBox"><div>
	<img src="/images/TraceLogger.png" />
	<span>Trace Logger</span>
</div></div>

### Basic Information ###

The trace logger displays the execution log of any of the supported CPUs.  It can display the last 30,000 instructions executed.
Additionally, it is also possible to log these instructions to the disk by using the `Start Logging` button.
Log files can rapidly grow in size (to several GBs worth of data in a few seconds), so it is recommended to log for the shortest amount of time needed.

### Display Options ###

This section configures which CPUs are logged, and allows you to customize the content of each row.

**Note**: Currently, only the main CPU (S-CPU) can have its format customized - all other CPUs use a preset format that cannot be changed.

Additional options:

* **Status Flag Format**: Offers a number of different ways to display the CPU's status flags.
* **Use Labels**: When enabled, addresses that match known labels will be replaced by their label instead.
* **Use Windows EOL**: When enabled, the log will use Windows-style end-of-line characters (LF+CR) instead of just LF.

### Custom Formatting ###

The trace logger's output can be customized by enabling the **Format Override** option and editing the format string.
See the help icon in the trace logger window for a summary of available tags and features.
