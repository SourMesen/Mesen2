using Mesen.GUI.Config;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class NesConfigViewModel : ViewModelBase
	{
		[Reactive] public NesConfig Config { get; set; }
		[Reactive] public UInt32[] Palette { get; set; }

		[Reactive] public bool ShowExpansionVolume { get; set; }
		[Reactive] public bool ShowColorIndexes { get; set; }

		[ObservableAsProperty] public bool IsDelayStereoEffect { get; set; }
		[ObservableAsProperty] public bool IsPanningStereoEffect { get; set; }
		[ObservableAsProperty] public bool IsCombStereoEffect { get; set; }
		
		public NesInputConfigViewModel Input { get; private set; }

		public NesConfigViewModel()
		{
			Config = ConfigManager.Config.Nes.Clone();
			Input = new NesInputConfigViewModel(Config);
			Palette = new UInt32[64] { 0xFF666666, 0xFF002A88, 0xFF1412A7, 0xFF3B00A4, 0xFF5C007E, 0xFF6E0040, 0xFF6C0600, 0xFF561D00, 0xFF333500, 0xFF0B4800, 0xFF005200, 0xFF004F08, 0xFF00404D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFADADAD, 0xFF155FD9, 0xFF4240FF, 0xFF7527FE, 0xFFA01ACC, 0xFFB71E7B, 0xFFB53120, 0xFF994E00, 0xFF6B6D00, 0xFF388700, 0xFF0C9300, 0xFF008F32, 0xFF007C8D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFF64B0FF, 0xFF9290FF, 0xFFC676FF, 0xFFF36AFF, 0xFFFE6ECC, 0xFFFE8170, 0xFFEA9E22, 0xFFBCBE00, 0xFF88D800, 0xFF5CE430, 0xFF45E082, 0xFF48CDDE, 0xFF4F4F4F, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFFC0DFFF, 0xFFD3D2FF, 0xFFE8C8FF, 0xFFFBC2FF, 0xFFFEC4EA, 0xFFFECCC5, 0xFFF7D8A5, 0xFFE4E594, 0xFFCFEF96, 0xFFBDF4AB, 0xFFB3F3CC, 0xFFB5EBF2, 0xFFB8B8B8, 0xFF000000, 0xFF000000 };

			this.WhenAnyValue(x => x.Config.StereoFilter).Select(x => x == StereoFilter.Delay).ToPropertyEx(this, x => x.IsDelayStereoEffect);
			this.WhenAnyValue(x => x.Config.StereoFilter).Select(x => x == StereoFilter.Panning).ToPropertyEx(this, x => x.IsPanningStereoEffect);
			this.WhenAnyValue(x => x.Config.StereoFilter).Select(x => x == StereoFilter.CombFilter).ToPropertyEx(this, x => x.IsCombStereoEffect);
		}
   }
}
