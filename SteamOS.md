Running Mesen through the Steam Deck's _Game Mode_ is possible with some caveats regarding rendering the UI.  
Due to Gamescope (SteamOS' compositor) not handling Avalonia UI's popups very well (a [solution](https://github.com/AvaloniaUI/Avalonia/pull/14366) is available but [has been reverted due to other issues](https://github.com/AvaloniaUI/Avalonia/pull/14573)), Mesen's menus for settings are not working through Gamescope unless running Mesen [through running KDE Plasma's Desktop through a script](https://www.reddit.com/r/SteamDeck/comments/zqgx9g/desktop_mode_within_gaming_mode_updated_for_new/).

 Installation instructions:
 * Download the **[Linux - AppImage](https://nightly.link/SourMesen/Mesen2/workflows/build/master/Mesen%20(Linux%20x64%20-%20AppImage).zip)** nightly build.
 * **Mark the AppImage as executable.** (right click > Properties > Permissions > Is executable)
 * Add the application as a non-Steam shortcut to be able to run it through Steam on both _Desktop Mode_ and _Game Mode_. (right click > Add to Steam)
 * Customise the non-Steam shortcut through Steam to your desire. (in Steam: search the AppImage's filename, right click > Properties; from there you can change the icon, shortcut name and launch options)
 * Create a folder called `Mesen.AppImage.Config` in the same directory where the AppImage is stored.
 * Run it the first time. When asking where to store the settings, choose the `Store the data in my user profile` option. ![254295455-88c1942d-b81f-48ee-a3a3-74b9f3ecd9b7-1](https://github.com/kevin-wijnen/Mesen2/assets/58944808/9f4ff1e3-4df6-4441-958b-ce96599ef69d)
 * Set up the controls as asked by Mesen.

**Due to Gamescope not rendering the UI menus, it is recommended to bind some keyboard shortcuts to L4/R4/L5/R5 (the Back Grip Buttons).** You can rebind controls in _Game Mode_ by clicking the Controller icon. You can save the layout by clicking the Cog icon (next to `Edit Layout`) > Export Layout > select `New Template` as the Export Type to use it across multiple shortcuts. 
It is recommended to:
* Bind `Control Key + O Key` to open the file picker for opening a game file.
* Bind `Escape Key` to pause emulation.
* Bind `F11` to enter in or out of fullscreen. 

**If sound does not work**, check if an audio device is chosen by Mesen. (in Mesen: Settings > Audio > General (Device))

**To make game-specific shortcuts**: Repeat the non-Steam shortcut step on the Mesen AppImage. Customise the new shortcut with a Launch Option (in Steam: right click > Properties; Launch Options). To find possible Launch Options, check the Command-line options menu (in Mesen: Help > Command-line options). When you want to supply a game with the shortcut, put the entire file location of the game in double quotes ("game-filepath") as the first part of the launch options. Add additional options (`--fullscreen` for example) _after_ the file location.
