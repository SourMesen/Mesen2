using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Text.Json;

namespace Mesen.Config
{
	public class CheatCodes
	{
		private static string FilePath { get { return Path.Combine(ConfigManager.CheatFolder, EmuApi.GetRomInfo().GetRomName() + ".json"); } }

		public List<CheatCode> Cheats { get; set; } = new List<CheatCode>();

		public static CheatCodes LoadCheatCodes()
		{
			return Deserialize(CheatCodes.FilePath);
		}

		private static CheatCodes Deserialize(string path)
		{
			CheatCodes cheats = new CheatCodes();

			if(File.Exists(path)) {
				try {
					cheats = (CheatCodes?)JsonSerializer.Deserialize(File.ReadAllText(path), typeof(CheatCodes), MesenSerializerContext.Default) ?? new CheatCodes();
				} catch { }
			}
			
			return cheats;
		}

		public void Save()
		{
			try {
				if(Cheats.Count > 0) {
					FileHelper.WriteAllText(CheatCodes.FilePath, JsonSerializer.Serialize(this, typeof(CheatCodes), MesenSerializerContext.Default));
				} else {
					if(File.Exists(CheatCodes.FilePath)) {
						File.Delete(CheatCodes.FilePath);
					}
				}
			} catch {
			}
		}

		public static void ApplyCheats()
		{
			if(ConfigManager.Config.Cheats.DisableAllCheats) {
				EmuApi.ClearCheats();
			} else {
				CheatCodes.ApplyCheats(LoadCheatCodes().Cheats);
			}
		}

		public static void ApplyCheats(IEnumerable<CheatCode> cheats)
		{
			List<InteropCheatCode> encodedCheats = new List<InteropCheatCode>();
			foreach(CheatCode cheat in cheats) {
				if(cheat.Enabled) {
					encodedCheats.AddRange(cheat.ToInteropCheats());
				}
			}

			EmuApi.SetCheats(encodedCheats.ToArray(), (UInt32)encodedCheats.Count);
		}
	}

	public class CheatCode : ViewModelBase
	{
		[Reactive] public string Description { get; set; } = "";
		[Reactive] public CheatType Type { get; set; }
		[Reactive] public bool Enabled { get; set; } = true;
		[Reactive] public string Codes { get; set; } = "";

		public List<InteropCheatCode> ToInteropCheats()
		{
			List<InteropCheatCode> encodedCheats = new List<InteropCheatCode>();
			foreach(string code in Codes.Split('\n', StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries)) {
				encodedCheats.Add(new InteropCheatCode(Type, code));
			}
			return encodedCheats;
		}

		public CheatCode Clone()
		{
			return new() {
				Description = Description,
				Type = Type,
				Enabled = Enabled,
				Codes = Codes,
			};
		}

		public void CopyFrom(CheatCode copy)
		{
			Description = copy.Description;
			Type = copy.Type;
			Enabled = copy.Enabled;
			Codes = String.Join(Environment.NewLine, copy.Codes.Split('\n', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries));
		}
	}
}
