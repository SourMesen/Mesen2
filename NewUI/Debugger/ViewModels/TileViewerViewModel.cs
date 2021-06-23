using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels
{
	public class TileViewerViewModel : ViewModelBase
	{
		private CpuType _cpuType { get; }
		private ConsoleType _consoleType { get; }

		[Reactive] public SnesMemoryType MemoryType { get; set; }
		[Reactive] public TileFormat TileFormat { get; set; }
		[Reactive] public TileLayout TileLayout { get; set; }
		[Reactive] public TileBackground TileBackground { get; set; }
		[Reactive] public bool ShowGrid { get; set; }
		[Reactive] public int SelectedPalette { get; set; }
		[Reactive] public int RowCount { get; set; }
		[Reactive] public int ColumnCount { get; set; }
		[Reactive] public int StartAddress { get; set; }
		[Reactive] public UInt32[] PaletteColors { get; set; } = Array.Empty<UInt32>();

		[ObservableAsProperty] public PaletteSelectionMode PaletteSelectionMode { get; }
		[ObservableAsProperty] public int AddressIncrement { get; }
		[ObservableAsProperty] public int MaximumAddress { get; }

		[Reactive] public Enum[] AvailableMemoryTypes { get; set; } = Array.Empty<Enum>();
		[Reactive] public Enum[] AvailableFormats { get; set; } = Array.Empty<Enum>();
		[Reactive] public bool ShowFormatDropdown { get; set; }

		//For designer
		public TileViewerViewModel() : this(CpuType.Cpu, ConsoleType.Snes) { }

		public TileViewerViewModel(CpuType cpuType, ConsoleType consoleType)
		{
			_cpuType = cpuType;
			_consoleType = consoleType;

			TileFormat = TileFormat.Bpp2;
			TileLayout = TileLayout.Normal;
			TileBackground = TileBackground.Default;
			ShowGrid = false;
			SelectedPalette = 0;
			StartAddress = 0;
			ColumnCount = 32;
			RowCount = 64;

			if(Design.IsDesignMode) {
				return;
			}

			AvailableMemoryTypes = Enum.GetValues<SnesMemoryType>().Where(t => DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();
			switch(_cpuType) {
				case CpuType.Cpu: 
					MemoryType = SnesMemoryType.VideoRam;
					AvailableFormats = new Enum[] { TileFormat.Bpp2, TileFormat.Bpp4, TileFormat.Bpp8, TileFormat.DirectColor, TileFormat.Mode7, TileFormat.Mode7DirectColor };
					break;

				case CpuType.Nes:
					MemoryType = AvailableMemoryTypes.Contains(SnesMemoryType.NesChrRom) ? SnesMemoryType.NesChrRom : SnesMemoryType.NesChrRam;
					AvailableFormats = new Enum[] { TileFormat.NesBpp2 };
					break;

				case CpuType.Gameboy:
					MemoryType = SnesMemoryType.GbVideoRam;
					AvailableFormats = new Enum[] { TileFormat.Bpp2 };
					break;
			}
			ShowFormatDropdown = AvailableFormats.Length > 1;

			this.WhenAnyValue(x => x.TileFormat).Select(x => {
				if(x == TileFormat.Bpp2 || x == TileFormat.NesBpp2) {
					return PaletteSelectionMode.FourColors;
				} else if(x == TileFormat.Bpp4) {
					return PaletteSelectionMode.SixteenColors;
				} else {
					return PaletteSelectionMode.None;
				}
			}).ToPropertyEx(this, x => x.PaletteSelectionMode);

			this.WhenAnyValue(x => x.ColumnCount, x => x.RowCount, x => x.TileFormat).Select(((int column, int row, TileFormat format) o) => {
				int bpp = 0;
				switch(o.format) {
					case TileFormat.Bpp2: bpp = 2; break;
					case TileFormat.Bpp4: bpp = 4; break;
					case TileFormat.DirectColor: bpp = 8; break;
					case TileFormat.Mode7: bpp = 16; break;
					case TileFormat.Mode7DirectColor: bpp = 16; break;
					case TileFormat.NesBpp2: bpp = 2; break;
					default: bpp = 8; break;
				}
				return o.column * o.row * 8 * 8 * bpp / 8;
			}).ToPropertyEx(this, x => x.AddressIncrement);

			this.WhenAnyValue(x => x.MemoryType).Select(memType => {
				return DebugApi.GetMemorySize(memType) - 1;
			}).ToPropertyEx(this, x => x.MaximumAddress);
		}

		public void UpdatePaletteColors()
		{
			PaletteColors = PaletteHelper.GetConvertedPalette(_cpuType, _consoleType);
		}
	}
}
