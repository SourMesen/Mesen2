using Avalonia.Media;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.Windows;
using ReactiveUI.Fody.Helpers;
using System;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class UpdatePromptViewModel : ViewModelBase
	{
		public Version LatestVersion { get; }
		public Version InstalledVersion { get; }
		public string Changelog { get; }

		public UpdatePromptViewModel(Version latest, string changelog)
		{
			LatestVersion = latest;
			Changelog = changelog;
			InstalledVersion = EmuApi.GetMesenVersion();
		}

		public static async Task<UpdatePromptViewModel> GetUpdateInformation()
		{
			//TODOv2 get latest version number & changelog online
			await Task.Run(() => { });
			return new UpdatePromptViewModel(new Version(2, 0, 0), "...");
		}

		public bool UpdateMesen()
		{
			//TODOv2 download latest version online
			string downloadPath = Path.Combine(ConfigManager.BackupFolder, "Mesen." + LatestVersion.ToString(3) + ".exe");
			File.Copy(Program.ExePath, downloadPath, true);
			return UpdateHelper.LaunchUpdate(downloadPath);
		}
	}
}
