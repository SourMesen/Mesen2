---
title: Event Viewer
weight: 13
chapter: false
---

## PPU View ##

<div class="imgBox"><div>
	<img src="/images/EventViewer.png" />
	<span>Event Viewer - PPU View</span>
</div></div>

The Event Viewer's PPU view allows you to visually check the timing at which various events (register read/writes, NMIs, IRQs, etc.) occur. This can be useful when trying to perform timing-critical mid-frame changes to the PPU, or to verify that PPU updates finish before vertical blank ends, etc.

The colors can be configured and it's also possible to define [breakpoints](/debugging/debugger.html#breakpoint-configuration) as marks to be shown on the Event Viewer. This is done by enabling a breakpoint's `Mark on Event Viewer` option.  This allows the event viewer to be used to display virtually any special event that needs to be tracked.

When the `Show previous frame's events` option is enabled, all events for the last full frame will be shown. Otherwise, only events that have occurred since the beginnning of the current frame will be shown.

## List View ##

<div class="imgBox"><div>
	<img src="/images/EventViewer_ListView.png" />
	<span>Event Viewer - List View</span>
</div></div>

The list view displays the same general information as the PPU view, but in a sortable list.
