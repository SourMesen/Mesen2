---
title: Tools
weight: 3
chapter: false
---

<div class="imgBox right"><div>
	<img src="/images/ToolsMenu.png" />
	<span>Tools Menu</span>
</div></div>

## Netplay ##

### Hosting a game ###

<div class="imgBox"><div>
	<img src="/images/NetplayHost.png" />
	<span>Server Configuration</span>
</div></div>

**Server name**: This name will be shown to clients when they connect.

**Port**: The port used for the communication.  Mesen-S will attempt to automatically port-forward this port on your router when you start the server -- if this fails, you will have to manually forward the port on your router to allow people outside of your LAN to connect to your server.

**Password** (optional): Any password entered here will need to be entered by clients when they try to connect -- can be left blank.

<div class="clear"></div>

### Connecting to a server ###

<div class="imgBox"><div>
	<img src="/images/NetplayConnect.png" />
	<span>Connect to...</span>
</div></div>

**Host**: The host name of the server you want to connect to.  This is usually an IP address but can also be a domain name.

**Port**: The port to connect to -- this must match the `Port` value used by the server's host.

**Password** (optional): Enter the password the host has set for the server here -- can be left blank.

Once you are connected to a server, you can select your controller via the **<kbd>Tools&rarr;Netplay&rarr;Select Controller</kbd>** menu.

## Movies ##

<div class="imgBox"><div>
	<img src="/images/MovieRecordingOptions.png" />
	<span>Movie Recording Options</span>
</div></div>

`Movies` are files that can be created by Mesen-S and played back within Mesen-S itself.  They are essentially a recording of the events that occurred during recording. To record an actual video file, see [Video Recorder](#video-recorder).

When you start recording, a configuration dialog is shown that allows you to select a number of options.

* **Save to**: The location where the movie will be saved. Press the **Browse** button to change it.
* **Record from**: Selects where the recording should start:
	* **Power on**: This will reset the game and start recording from the start. Your save data (.sav files) will be excluded from the movie file - after the reset, you will start the game as if it had never been run yet.
	* **Power on, with save data**: This will reset the game and start recording from the start. Your save data (.sav files, etc.) will be included in the movie file.
	* **Current state**: This will start recording from the current state -- in this case, the movie file will contain a save state.
* **Author**: The movie's author (optional) - this will be saved in the movie file.
* **Description**: A description of the movie's content (optional) - this will be saved in the movie file.

To play a movie file, select it via the **<kbd>Tools&rarr;Movies&rarr;Play</kbd>** command.

## Cheats ##

<div class="imgBox"><div>
	<img src="/images/CheatList.png" />
	<span>Cheats Window</span>
</div></div>

Mesen-S supports cheats in Game Genie and Pro Action Replay formats.  

It is also possible to import cheats from the built-in [Cheat Database](#cheat-database).

**To add a new cheat code**, click the `Add Cheat` button.  
**To edit a cheat code**, double-click on it in the list.  
**To delete a cheat code**, select it from the list and click the `Delete` button.

**To import cheats**, click the `Cheat Database` button.  

**To disable a specific cheat**, uncheck it from the list.  
**To disable all cheats**, check the `Disable all cheats` option.

<div class="clear"></div>

### Adding/Editing Cheats ###

<div class="imgBox"><div>
	<img src="/images/EditCheat.png" />
	<span>Edit Cheat</span>
</div></div>

You must give each cheat a name, which will be used to display it in the cheats list.

The `Codes` section lets you enter the actual cheat codes, in either Game Genie or Pro Action Replay format.

<div class="clear"></div>

### Cheat Database ###

<div class="imgBox"><div>
	<img src="/images/CheatDbImport.png" />
	<span>Import cheats from the built-in cheat database</span>
</div></div>

To import from the cheats database, click `Cheat Database`.  

In the next dialog, select the game for which you want to import cheats.  You can type in the `Search` bar at the top to filter the game list.  Once you've selected a game, press OK -- this will import all cheats for that game into the cheats list.  You can then manually enable any cheat you want to use.

By default, the game that is currently loaded will be selected for you.  Having no game selected when the dialog opens indicates that there are no built-in cheats available for the game that is currently running.

## Sound Recorder ##

The sound recorder allows you to record uncompressed `.wav` files.  The `.wav` file will use the exact same output settings as Mesen-S' [audio options](/configuration/audio.html) -- this means the sample rate will match the current sample rate, and that any sound modification (volume, panning, equalizer or effects) will also be applied to the `.wav` files.

To start recording, use the **<kbd>Tools&rarr;Sound Recorder&rarr;Record</kbd>** command.  
To stop an on-going recording, use the **<kbd>Tools&rarr;Sound Recorder&rarr;Stop Recording</kbd>** command.

## Video Recorder ##

<div class="imgBox"><div>
	<img src="/images/VideoRecording.png" />
	<span>Video Recorder</span>
</div></div>

Much like the sound recorder, the video recorder allows you to record `.avi` files.

Before you start a recording, you can select where to save the `.avi` file and which video codec to use.  All video codecs are lossless codecs -- the only reason to reduce the compression level to a lower level is to improve performance in the event your computer is having a hard time recording the video and running the emulation at its normal speed at the same time.

To start recording, use the **<kbd>Tools&rarr;Video Recorder&rarr;Record</kbd>** command.  
To stop an on-going recording, use the **<kbd>Tools&rarr;Video Recorder&rarr;Stop Recording</kbd>** command.

## Log Window ##

<div class="imgBox"><div>
	<img src="/images/LogWindow.png" />
	<span>Log Window</span>
</div></div>

The Log Window displays various information -- mostly about the roms you load.  
It is also sometimes used as a way to log errors or warnings.
