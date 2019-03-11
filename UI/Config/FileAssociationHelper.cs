using System;
using System.Collections.Generic;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Windows.Forms;
using Mesen.GUI.Config;
using Microsoft.Win32;

namespace Mesen.GUI.Config
{
	class FileAssociationHelper
	{
		static private string CreateMimeType(string mimeType, string extension, string description, List<string> mimeTypes, bool addType)
		{
			string baseFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), ".local", "share", "mime", "packages");
			if(!Directory.Exists(baseFolder)) {
				Directory.CreateDirectory(baseFolder);
			}
			string filename = Path.Combine(baseFolder, mimeType + ".xml");
			
			if(addType) {
				File.WriteAllText(filename,
					"<?xml version=\"1.0\" encoding=\"utf-8\"?>" + Environment.NewLine +
					"<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">" + Environment.NewLine +
					"\t<mime-type type=\"application/" + mimeType + "\">" + Environment.NewLine +
					"\t\t<glob-deleteall/>" + Environment.NewLine +
					"\t\t<glob pattern=\"*." + extension + "\"/>" + Environment.NewLine +
					"\t\t<comment>" + description + "</comment>" + Environment.NewLine +
					"\t\t<icon>MesenIcon</icon>" + Environment.NewLine +
					"\t</mime-type>" + Environment.NewLine +
					"</mime-info>" + Environment.NewLine);

				mimeTypes.Add(mimeType);
			} else if(File.Exists(filename)) {
				try {
					File.Delete(filename);
				} catch { }
			}
			return mimeType;
		}

		static public void ConfigureLinuxMimeTypes()
		{
			PreferencesConfig cfg = ConfigManager.Config.Preferences;

			string baseFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), ".local", "share");
			string desktopFolder = Path.Combine(baseFolder, "applications");
			string mimeFolder = Path.Combine(baseFolder, "mime");
			string iconFolder = Path.Combine(baseFolder, "icons");
			if(!Directory.Exists(mimeFolder)) {
				Directory.CreateDirectory(mimeFolder);
			}
			if(!Directory.Exists(iconFolder)) {
				Directory.CreateDirectory(iconFolder);
			}
			if(!Directory.Exists(desktopFolder)) {
				Directory.CreateDirectory(desktopFolder);
			}


			//Use a GUID to get a unique filename and then delete old files to force a reset of file associations
			//Otherwise they are sometimes not refreshed properly
			string desktopFilename = "mesen-s." + Guid.NewGuid().ToString() + ".desktop";
			string desktopFile = Path.Combine(desktopFolder, desktopFilename);

			foreach(string file in Directory.GetFiles(desktopFolder, "mesen-s.*.desktop")) {
				if(File.Exists(file)) {
					try {
						File.Delete(file);
					} catch { }
				}
			}

			List<string> mimeTypes = new List<string>();
			CreateMimeType("x-mesen-nes", "sfc", "SNES Rom", mimeTypes, cfg.AssociateRomFiles);
			CreateMimeType("x-mesen-nes", "smc", "SNES Rom", mimeTypes, cfg.AssociateRomFiles);
			CreateMimeType("x-mesen-nes", "swc", "SNES Rom", mimeTypes, cfg.AssociateRomFiles);
			CreateMimeType("x-mesen-nes", "fig", "SNES Rom", mimeTypes, cfg.AssociateRomFiles);
			CreateMimeType("x-mesen-mst", "mst", "Mesen-S Save State", mimeTypes, cfg.AssociateMssFiles);
			CreateMimeType("x-mesen-mmo", "mmo", "Mesen-S Movie File", mimeTypes, cfg.AssociateMsmFiles);

			//Icon used for shortcuts
			//TOOD
			//Mesen.GUI.Properties.Resources.MesenIcon.Save(Path.Combine(iconFolder, "MesenSIcon.png"), ImageFormat.Png);

			CreateShortcutFile(desktopFile, mimeTypes);

			//Update databases
			try {
				System.Diagnostics.Process.Start("update-mime-database", mimeFolder).WaitForExit();
				System.Diagnostics.Process.Start("update-desktop-database", desktopFolder);
			} catch {
				try {
					EmuApi.WriteLogEntry("An error occurred while updating file associations");
				} catch {
					//For some reason, Mono crashes when trying to call this if libMesenCore.dll was not already next to the .exe before the process starts?
					//This causes a "MesenCore.dll not found" popup, so catch it here and ignore it.
				}
			}
		}

		static public void CreateShortcutFile(string filename, List<string> mimeTypes = null)
		{
			string content = 
				"[Desktop Entry]" + Environment.NewLine +
				"Type=Application" + Environment.NewLine +
				"Name=Mesen-S" + Environment.NewLine +
				"Comment=SNES Emulator" + Environment.NewLine +
				"Keywords=game;snes;super;famicom;emulator;emu;ファミコン;nintendo" + Environment.NewLine +
				"Categories=GNOME;GTK;Game;Emulator;" + Environment.NewLine;
			if(mimeTypes != null) {
				content += "MimeType=" + string.Join(";", mimeTypes.Select(type => "application/" + type)) + Environment.NewLine;
			}
			content +=
				"Exec=mono " + System.Reflection.Assembly.GetEntryAssembly().Location + " %f" + Environment.NewLine +
				"NoDisplay=false" + Environment.NewLine +
				"StartupNotify=true" + Environment.NewLine +
				"Icon=MesenSIcon" + Environment.NewLine;

			File.WriteAllText(filename, content);
		}

		static public void UpdateFileAssociation(string extension, bool associate)
		{
			string key = @"HKEY_CURRENT_USER\Software\Classes\." + extension;
			if(associate) {
				Registry.SetValue(@"HKEY_CURRENT_USER\Software\Classes\Mesen-S\shell\open\command", null, Application.ExecutablePath + " \"%1\"");
				Registry.SetValue(key, null, "Mesen-S");
			} else {
				object regKey = Registry.GetValue(key, null, "");
				if(regKey != null && regKey.Equals("Mesen-S")) {
					Registry.SetValue(key, null, "");
				}
			}
		}
	}
}