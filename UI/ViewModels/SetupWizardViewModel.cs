using Avalonia.Controls;
using Mesen.Config;
using Mesen.Utilities;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Diagnostics;
using System.IO;

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
		[Reactive] public bool IsOsx { get; set; } = OperatingSystem.IsMacOS();

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
			if(OperatingSystem.IsMacOS()) {
				//TODO OSX
				return;
			}
			
			if(OperatingSystem.IsWindows()) {
				string linkPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory), "Mesen.url");
				FileHelper.WriteAllText(linkPath,
					"[InternetShortcut]" + Environment.NewLine +
					"URL=file:///" + Program.ExePath + Environment.NewLine +
					"IconIndex=0" + Environment.NewLine +
					"IconFile=" + Program.ExePath.Replace('\\', '/') + Environment.NewLine
				);
			} else {
				string shortcutFile = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Desktop), "mesen.desktop");
				FileAssociationHelper.CreateLinuxShortcutFile(shortcutFile);
				Process.Start("chmod", "744 " + shortcutFile);
			}
		}
	}
}
