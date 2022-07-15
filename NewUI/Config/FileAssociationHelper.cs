using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using Microsoft.Win32;

namespace Mesen.Config
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
				FileHelper.WriteAllText(filename,
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
			CreateMimeType("x-mesen_s-sfc", "sfc", "SNES Rom", mimeTypes, cfg.AssociateSnesRomFiles);
			CreateMimeType("x-mesen_s-smc", "smc", "SNES Rom", mimeTypes, cfg.AssociateSnesRomFiles);
			CreateMimeType("x-mesen_s-swc", "swc", "SNES Rom", mimeTypes, cfg.AssociateSnesRomFiles);
			CreateMimeType("x-mesen_s-fig", "fig", "SNES Rom", mimeTypes, cfg.AssociateSnesRomFiles);
			CreateMimeType("x-mesen_s-bs", "bs", "BS-X Memory Pack", mimeTypes, cfg.AssociateSnesRomFiles);
			CreateMimeType("x-mesen_s-spc", "spc", "SPC Sound File", mimeTypes, cfg.AssociateSnesMusicFiles);

			CreateMimeType("x-mesen-nes", "nes", "NES ROM", mimeTypes, cfg.AssociateNesRomFiles);
			CreateMimeType("x-mesen-fds", "fds", "FDS ROM", mimeTypes, cfg.AssociateNesRomFiles);
			CreateMimeType("x-mesen-studybox", "studybox", "Studybox ROM (Famicom)", mimeTypes, cfg.AssociateNesRomFiles);
			CreateMimeType("x-mesen-unif", "unf", "NES ROM", mimeTypes, cfg.AssociateNesRomFiles);

			CreateMimeType("x-mesen-nsf", "nsf", "Nintendo Sound File", mimeTypes, cfg.AssociateNesMusicFiles);
			CreateMimeType("x-mesen-nsfe", "nsfe", "Nintendo Sound File (extended)", mimeTypes, cfg.AssociateNesMusicFiles);

			CreateMimeType("x-mesen-gb", "gb", "Game Boy ROM", mimeTypes, cfg.AssociateGbRomFiles);
			CreateMimeType("x-mesen-gbc", "gbc", "Game Boy Color ROM", mimeTypes, cfg.AssociateGbRomFiles);
			CreateMimeType("x-mesen-gbs", "gbs", "Game Boy Sound File", mimeTypes, cfg.AssociateGbMusicFiles);

			CreateMimeType("x-mesen-pce", "pce", "PC Engine ROM", mimeTypes, cfg.AssociatePceRomFiles);

			//Icon used for shortcuts
			//TOOD
			//Mesen.GUI.Properties.Resources.MesenIcon.Save(Path.Combine(iconFolder, "MesenSIcon.png"), ImageFormat.Png);

			CreateLinuxShortcutFile(desktopFile, mimeTypes);

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

		static public void CreateLinuxShortcutFile(string filename, List<string>? mimeTypes = null)
		{
			ProcessModule? mainModule = Process.GetCurrentProcess().MainModule;
			if(mainModule == null) {
				return;
			}

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
				"Exec=mono " + mainModule.FileName + " %f" + Environment.NewLine +
				"NoDisplay=false" + Environment.NewLine +
				"StartupNotify=true" + Environment.NewLine +
				"Icon=MesenSIcon" + Environment.NewLine;

			FileHelper.WriteAllText(filename, content);
		}

		static public void UpdateFileAssociation(string extension, bool associate)
		{
			if(!RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) {
				return;
			}

			string key = @"HKEY_CURRENT_USER\Software\Classes\." + extension;
			if(associate) {
				ProcessModule? mainModule = Process.GetCurrentProcess().MainModule;
				if(mainModule != null) {
					Registry.SetValue(@"HKEY_CURRENT_USER\Software\Classes\Mesen\shell\open\command", null, mainModule.FileName + " \"%1\"");
					Registry.SetValue(key, null, "Mesen");
				}
			} else {
				object? regKey = Registry.GetValue(key, null, "");
				if(regKey != null && regKey.Equals("Mesen")) {
					Registry.SetValue(key, null, "");
				}
			}
		}
	}
}