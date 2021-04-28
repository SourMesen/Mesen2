using Mesen.GUI.Config;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class NesConfigViewModel : ViewModelBase
	{
		[Reactive] public NesConfig Config { get; set; }

		[Reactive] public bool ShowExpansionVolume { get; set; }
		[Reactive] public bool ShowColorIndexes { get; set; }

		[ObservableAsProperty] public bool IsDelayStereoEffect { get; set; }
		[ObservableAsProperty] public bool IsPanningStereoEffect { get; set; }
		[ObservableAsProperty] public bool IsCombStereoEffect { get; set; }
		
		public NesInputConfigViewModel Input { get; private set; }

		//For designer
		public NesConfigViewModel() : this(new PreferencesConfig()) { }

		public NesConfigViewModel(PreferencesConfig preferences)
		{
			Config = ConfigManager.Config.Nes.Clone();
			Input = new NesInputConfigViewModel(Config, preferences);

			this.WhenAnyValue(x => x.Config.StereoFilter).Select(x => x == StereoFilter.Delay).ToPropertyEx(this, x => x.IsDelayStereoEffect);
			this.WhenAnyValue(x => x.Config.StereoFilter).Select(x => x == StereoFilter.Panning).ToPropertyEx(this, x => x.IsPanningStereoEffect);
			this.WhenAnyValue(x => x.Config.StereoFilter).Select(x => x == StereoFilter.CombFilter).ToPropertyEx(this, x => x.IsCombStereoEffect);
		}

		public void LoadPaletteFile(string filename)
		{
			using(FileStream paletteFile = File.OpenRead(filename)) {
				byte[] paletteFileData = new byte[512 * 3];
				int byteCount = paletteFile.Read(paletteFileData, 0, 512 * 3);
				if(byteCount == 64 * 3 || byteCount == 512 * 3) {
					UInt32[] paletteData = new UInt32[byteCount / 3];
					for(int i = 0; i < byteCount; i += 3) {
						paletteData[i / 3] = ((UInt32)0xFF000000 | (UInt32)paletteFileData[i + 2] | (UInt32)(paletteFileData[i + 1] << 8) | (UInt32)(paletteFileData[i] << 16));
					}
					Config.UserPalette = paletteData;
				}
				paletteFile.Close();
			}
		}

		public void ExportPalette(string filename)
		{
			List<byte> bytePalette = new List<byte>();
			foreach(UInt32 value in Config.UserPalette) {
				bytePalette.Add((byte)(value >> 16 & 0xFF));
				bytePalette.Add((byte)(value >> 8 & 0xFF));
				bytePalette.Add((byte)(value & 0xFF));
			}
			File.WriteAllBytes(filename, bytePalette.ToArray());
		}
	}
}
