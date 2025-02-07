using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using Avalonia.Threading;
using Mesen.Interop;
using Mesen.Utilities;
using Microsoft.Win32;

namespace Mesen.Config
{
	class FileAssociationHelper
	{
		static private string CreateMimeType(string mimeType, string extension, string description, List<string> mimeTypes, bool addType)
		{
			string baseFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "mime", "packages");
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

		static public void UpdateLinuxFileAssociations()
		{
			PreferencesConfig cfg = ConfigManager.Config.Preferences;

			string baseFolder = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));
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

			List<string> mimeTypes = new List<string>();
			CreateMimeType("x-mesen-sfc", "sfc", "SNES Rom", mimeTypes, cfg.AssociateSnesRomFiles);
			CreateMimeType("x-mesen-smc", "smc", "SNES Rom", mimeTypes, cfg.AssociateSnesRomFiles);
			CreateMimeType("x-mesen-swc", "swc", "SNES Rom", mimeTypes, cfg.AssociateSnesRomFiles);
			CreateMimeType("x-mesen-fig", "fig", "SNES Rom", mimeTypes, cfg.AssociateSnesRomFiles);
			CreateMimeType("x-mesen-bs", "bs", "BS-X Memory Pack", mimeTypes, cfg.AssociateSnesRomFiles);
			CreateMimeType("x-mesen-spc", "spc", "SPC Sound File", mimeTypes, cfg.AssociateSnesMusicFiles);

			CreateMimeType("x-mesen-nes", "nes", "NES ROM", mimeTypes, cfg.AssociateNesRomFiles);
			CreateMimeType("x-mesen-fds", "fds", "FDS ROM", mimeTypes, cfg.AssociateNesRomFiles);
			CreateMimeType("x-mesen-qd", "qd", "FDS ROM (QD format)", mimeTypes, cfg.AssociateNesRomFiles);
			CreateMimeType("x-mesen-studybox", "studybox", "Studybox ROM (Famicom)", mimeTypes, cfg.AssociateNesRomFiles);
			CreateMimeType("x-mesen-unif", "unf", "NES ROM", mimeTypes, cfg.AssociateNesRomFiles);

			CreateMimeType("x-mesen-nsf", "nsf", "Nintendo Sound File", mimeTypes, cfg.AssociateNesMusicFiles);
			CreateMimeType("x-mesen-nsfe", "nsfe", "Nintendo Sound File (extended)", mimeTypes, cfg.AssociateNesMusicFiles);

			CreateMimeType("x-mesen-gb", "gb", "Game Boy ROM", mimeTypes, cfg.AssociateGbRomFiles);
			CreateMimeType("x-mesen-gbx", "gbx", "Game Boy ROM", mimeTypes, cfg.AssociateGbRomFiles);
			CreateMimeType("x-mesen-gbc", "gbc", "Game Boy Color ROM", mimeTypes, cfg.AssociateGbRomFiles);
			CreateMimeType("x-mesen-gbs", "gbs", "Game Boy Sound File", mimeTypes, cfg.AssociateGbMusicFiles);
			CreateMimeType("x-mesen-gba", "gba", "Game Boy Advance ROM", mimeTypes, cfg.AssociateGbaRomFiles);

			CreateMimeType("x-mesen-pce", "pce", "PC Engine ROM", mimeTypes, cfg.AssociatePceRomFiles);
			CreateMimeType("x-mesen-sgx", "sgx", "PC Engine SuperGrafx ROM", mimeTypes, cfg.AssociatePceRomFiles);
			CreateMimeType("x-mesen-hes", "hes", "PC Engine Sound File", mimeTypes, cfg.AssociatePceMusicFiles);

			CreateMimeType("x-mesen-sms", "sms", "Master System ROM", mimeTypes, cfg.AssociateSmsRomFiles);
			CreateMimeType("x-mesen-gg", "gg", "Game Gear ROM", mimeTypes, cfg.AssociateGameGearRomFiles);
			CreateMimeType("x-mesen-sg", "sg", "SG-1000 ROM", mimeTypes, cfg.AssociateSgRomFiles);
			CreateMimeType("x-mesen-col", "col", "ColecoVision ROM", mimeTypes, cfg.AssociateCvRomFiles);
			
			CreateMimeType("x-mesen-ws", "ws", "WonderSwan ROM", mimeTypes, cfg.AssociateWsRomFiles);
			CreateMimeType("x-mesen-wsc", "wsc", "WonderSwan Color ROM", mimeTypes, cfg.AssociateWsRomFiles);

			//Icon used for shortcuts
			ImageUtilities.BitmapFromAsset("Assets/MesenIcon.png").Save(Path.Combine(iconFolder, "MesenIcon.png"));

			string desktopFile = Path.Combine(desktopFolder, "mesen.desktop");
			if(!File.Exists(desktopFile)) {
				CreateLinuxShortcutFile(desktopFile, mimeTypes);
			} else {
				UpdateLinuxShortcutFileMimeTypes(desktopFile, mimeTypes);
			}

			//Update databases
			try {
				Process.Start("update-mime-database", mimeFolder).WaitForExit();
				Process.Start("update-desktop-database", desktopFolder);
			} catch {
				try {
					EmuApi.WriteLogEntry("An error occurred while updating file associations");
				} catch { }
			}
		}

		private static void UpdateLinuxShortcutFileMimeTypes(string desktopFile, List<string> mimeTypes)
		{
			string? content = FileHelper.ReadAllText(desktopFile);

			if(content != null) {
				List<string> lines = new List<string>(content.Split(Environment.NewLine));
				bool replaced = false;
				for(int i = 0; i < lines.Count; i++) {
					if(lines[i].Trim().StartsWith("MimeType=")) {
						lines[i] = "MimeType=" + string.Join(";", mimeTypes.Select(type => "application/" + type));
						replaced = true;
					}
				}

				if(!replaced) {
					lines.Add("MimeType=" + string.Join(";", mimeTypes.Select(type => "application/" + type)));
				}

				FileHelper.WriteAllText(desktopFile, string.Join(Environment.NewLine, lines), new UTF8Encoding(false));
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
				"Name=Mesen" + Environment.NewLine +
				"Comment=Emulator" + Environment.NewLine +
				"Keywords=game;emulator;emu" + Environment.NewLine +
				"Categories=GNOME;GTK;Game;Emulator;" + Environment.NewLine +
				"Exec=" + mainModule.FileName + " %f" + Environment.NewLine +
				"NoDisplay=false" + Environment.NewLine +
				"StartupNotify=true" + Environment.NewLine +
				"Icon=MesenIcon" + Environment.NewLine;
			
			if(mimeTypes != null) {
				content += "MimeType=" + string.Join(";", mimeTypes.Select(type => "application/" + type)) + Environment.NewLine;
			}

			FileHelper.WriteAllText(filename, content, new UTF8Encoding(false));
		}

		static public void UpdateFileAssociations()
		{
			try {
				if(OperatingSystem.IsWindows()) {
					FileAssociationHelper.UpdateWindowsFileAssociations();
				} else if(OperatingSystem.IsLinux()) {
					FileAssociationHelper.UpdateLinuxFileAssociations();
				}
			} catch(Exception ex) {
				Dispatcher.UIThread.Post(() => {
					MesenMsgBox.ShowException(ex);
				});
			}
		}

		static public void UpdateWindowsFileAssociations()
		{
			PreferencesConfig cfg = ConfigManager.Config.Preferences;
			FileAssociationHelper.UpdateFileAssociation("sfc", cfg.AssociateSnesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("smc", cfg.AssociateSnesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("swc", cfg.AssociateSnesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("fig", cfg.AssociateSnesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("bs", cfg.AssociateSnesRomFiles);

			FileAssociationHelper.UpdateFileAssociation("spc", cfg.AssociateSnesMusicFiles);

			FileAssociationHelper.UpdateFileAssociation("nes", cfg.AssociateNesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("fds", cfg.AssociateNesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("qd", cfg.AssociateNesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("unf", cfg.AssociateNesRomFiles);
			FileAssociationHelper.UpdateFileAssociation("studybox", cfg.AssociateNesRomFiles);

			FileAssociationHelper.UpdateFileAssociation("nsf", cfg.AssociateNesMusicFiles);
			FileAssociationHelper.UpdateFileAssociation("nsfe", cfg.AssociateNesMusicFiles);

			FileAssociationHelper.UpdateFileAssociation("gb", cfg.AssociateGbRomFiles);
			FileAssociationHelper.UpdateFileAssociation("gbx", cfg.AssociateGbRomFiles);
			FileAssociationHelper.UpdateFileAssociation("gbc", cfg.AssociateGbRomFiles);
			FileAssociationHelper.UpdateFileAssociation("gbs", cfg.AssociateGbMusicFiles);
			
			FileAssociationHelper.UpdateFileAssociation("gba", cfg.AssociateGbaRomFiles);

			FileAssociationHelper.UpdateFileAssociation("pce", cfg.AssociatePceRomFiles);
			FileAssociationHelper.UpdateFileAssociation("sgx", cfg.AssociatePceRomFiles);
			FileAssociationHelper.UpdateFileAssociation("hes", cfg.AssociatePceMusicFiles);

			FileAssociationHelper.UpdateFileAssociation("sms", cfg.AssociateSmsRomFiles);
			FileAssociationHelper.UpdateFileAssociation("gg", cfg.AssociateGameGearRomFiles);
			FileAssociationHelper.UpdateFileAssociation("sg", cfg.AssociateSgRomFiles);
			FileAssociationHelper.UpdateFileAssociation("col", cfg.AssociateCvRomFiles);
		
			FileAssociationHelper.UpdateFileAssociation("ws", cfg.AssociateWsRomFiles);
			FileAssociationHelper.UpdateFileAssociation("wsc", cfg.AssociateWsRomFiles);
		}

		static private void UpdateFileAssociation(string extension, bool associate)
		{
			if(!OperatingSystem.IsWindows()) {
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