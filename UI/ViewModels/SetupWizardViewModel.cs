using Avalonia.Controls;
using Mesen.Config;
using Mesen.Utilities;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Mesen.ViewModels
{
	public class SetupWizardViewModel : ViewModelBase
	{
		[Reactive] public bool StoreInUserProfile { get; set; } = true;

		[Reactive] public bool EnableXboxMappings { get; set; } = true;
		[Reactive] public bool EnablePsMappings { get; set; }
		[Reactive] public bool EnableWasdMappings { get; set; }
		[Reactive] public bool EnableArrowMappings { get; set; } = true;

		[Reactive] public string InstallLocation { get; set; }

		[Reactive] public bool CreateShortcut { get; set; } = true;
		[Reactive] public bool CheckForUpdates { get; set; } = true;
		[Reactive] public bool IsOsx { get; set; } = RuntimeInformation.IsOSPlatform(OSPlatform.OSX);

		public SetupWizardViewModel()
		{
			InstallLocation = ConfigManager.DefaultDocumentsFolder;

			this.WhenAnyValue(x => x.StoreInUserProfile).Subscribe(x => {
				if(StoreInUserProfile) {
					InstallLocation = ConfigManager.DefaultDocumentsFolder;
				} else {
					InstallLocation = ConfigManager.DefaultPortableFolder;
				}
			});

			this.WhenAnyValue(x => x.EnableWasdMappings).Subscribe(x => {
				if(x) {
					EnableArrowMappings = false;
				}
			});

			this.WhenAnyValue(x => x.EnableArrowMappings).Subscribe(x => {
				if(x) {
					EnableWasdMappings = false;
				}
			});
		}

		public bool Confirm(Window parent)
		{
			string targetFolder = StoreInUserProfile ? ConfigManager.DefaultDocumentsFolder : ConfigManager.DefaultPortableFolder;
			string testFile = Path.Combine(targetFolder, "test.txt");
			try {
				if(!Directory.Exists(targetFolder)) {
					Directory.CreateDirectory(targetFolder);
				}
				File.WriteAllText(testFile, "test");
				File.Delete(testFile);
				InitializeConfig();
				if(CreateShortcut) {
					CreateShortcutFile();
				}
				return true;
			} catch(Exception ex) {
				MesenMsgBox.Show(parent, "CannotWriteToFolder", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
			}

			return false;
		}

		private void InitializeConfig()
		{
			ConfigManager.CreateConfig(!StoreInUserProfile);
			DefaultKeyMappingType mappingType = DefaultKeyMappingType.None;
			if(EnableXboxMappings) {
				mappingType |= DefaultKeyMappingType.Xbox;
			}
			if(EnablePsMappings) {
				mappingType |= DefaultKeyMappingType.Ps4;
			}
			if(EnableWasdMappings) {
				mappingType |= DefaultKeyMappingType.WasdKeys;
			}
			if(EnableArrowMappings) {
				mappingType |= DefaultKeyMappingType.ArrowKeys;
			}

			ConfigManager.Config.DefaultKeyMappings = mappingType;
			ConfigManager.Config.Preferences.AutomaticallyCheckForUpdates = CheckForUpdates;
			ConfigManager.Config.Save();
		}

		private void CreateShortcutFile()
		{
			if(RuntimeInformation.IsOSPlatform(OSPlatform.OSX)) {
				//TODO OSX
				return;
			}
			
			if(RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) {
				Type? t = Type.GetTypeFromCLSID(new Guid("72C24DD5-D70A-438B-8A42-98424B88AFB8"));
				if(t == null) {
					return;
				}

				dynamic? shell = Activator.CreateInstance(t);
				if(shell == null) {
					return;
				}

				try {
					string linkPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory), "Mesen.lnk");
					var lnk = shell.CreateShortcut(linkPath);
					try {
						lnk.TargetPath = Program.ExePath;
						lnk.IconLocation = Program.ExePath + ", 0";
						lnk.Save();
					} finally {
						Marshal.FinalReleaseComObject(lnk);
					}
				} finally {
					Marshal.FinalReleaseComObject(shell);
				}
			} else {
				string shortcutFile = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Desktop), "mesen.desktop");
				FileAssociationHelper.CreateLinuxShortcutFile(shortcutFile);
				Process.Start("chmod", "744 " + shortcutFile);
			}
		}
	}
}
