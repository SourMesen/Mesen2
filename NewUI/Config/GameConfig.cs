using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class GameConfig : BaseConfig<AudioPlayerConfig>
	{
		[Reactive] public UInt32 DipSwitches { get; set; } = 0;

		[Reactive] public bool OverrideOverscan { get; set; } = false;
		[Reactive] public UInt32 OverscanLeft { get; set; } = 0;
		[Reactive] public UInt32 OverscanRight { get; set; } = 0;
		[Reactive] public UInt32 OverscanTop { get; set; } = 0;
		[Reactive] public UInt32 OverscanBottom { get; set; } = 0;

		public void ApplyConfig()
		{
			ConfigApi.SetGameConfig(new InteropGameConfig() {
				DipSwitches = DipSwitches,
				OverrideOverscan = OverrideOverscan,
				OverscanLeft = OverscanLeft,
				OverscanRight = OverscanRight,
				OverscanTop = OverscanTop,
				OverscanBottom = OverscanBottom,
			});
		}

		public static GameConfig LoadGameConfig(string romName)
		{
			string path = Path.Combine(ConfigManager.GameConfigFolder, romName + ".json");
			if(File.Exists(path)) {
				string? fileData = FileHelper.ReadAllText(path);
				if(fileData != null) {
					try {
						return JsonSerializer.Deserialize<GameConfig>(fileData, JsonHelper.Options) ?? new();
					} catch { }
				}
			}
			return new();
		}

		public void Save()
		{
			string romName = MainWindowViewModel.Instance.RomInfo.GetRomName();
			string path = Path.Combine(ConfigManager.GameConfigFolder, romName + ".json");
			FileHelper.WriteAllText(path, JsonSerializer.Serialize(this, JsonHelper.Options));
		}
	}

	public struct InteropGameConfig
	{
		public UInt32 DipSwitches;
		
		[MarshalAs(UnmanagedType.I1)] public bool OverrideOverscan;
		public UInt32 OverscanLeft;
		public UInt32 OverscanRight;
		public UInt32 OverscanTop;
		public UInt32 OverscanBottom;
	}
}