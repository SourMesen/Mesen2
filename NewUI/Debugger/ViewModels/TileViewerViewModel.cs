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
		public CpuType CpuType { get; }
		public ConsoleType ConsoleType { get; }

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
			CpuType = cpuType;
			ConsoleType = consoleType;

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
			switch(CpuType) {
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

		private uint To8Bit(int color)
		{
			return (uint)((color << 3) + (color >> 2));
		}

		public uint ToArgb(int rgb555)
		{
			uint b = To8Bit(rgb555 >> 10);
			uint g = To8Bit((rgb555 >> 5) & 0x1F);
			uint r = To8Bit(rgb555 & 0x1F);

			return (0xFF000000 | (r << 16) | (g << 8) | b);
		}

		public void UpdatePaletteColors()
		{
			switch(CpuType) {
				case CpuType.Cpu: {
					byte[] cgram = DebugApi.GetMemoryState(SnesMemoryType.CGRam);
					UInt32[] colors = new UInt32[256];
					for(int i = 0; i < 256; i++) {
						colors[i] = ToArgb(cgram[i * 2] | cgram[i * 2 + 1] << 8);
					}
					PaletteColors = colors;
					break;
				}

				case CpuType.Nes: {
					byte[] cgram = DebugApi.GetMemoryState(SnesMemoryType.NesPaletteRam);
					UInt32[] colors = new UInt32[32];
					for(int i = 0; i < 32; i++) {
						colors[i] = ConfigManager.Config.Nes.UserPalette[cgram[i]];
					}
					PaletteColors = colors;
					break;
				}

				case CpuType.Gameboy: {
					GbPpuState ppu = DebugApi.GetPpuState<GbPpuState>(CpuType.Gameboy);
					if(ConsoleType == ConsoleType.GameboyColor) {
						UInt32[] colors = new UInt32[64];
						for(int i = 0; i < 32; i++) {
							colors[i] = ToArgb(ppu.CgbBgPalettes[i]);
						}

						for(int i = 0; i < 32; i++) {
							colors[i+32] = ToArgb(ppu.CgbObjPalettes[i]);
						}
						PaletteColors = colors;
					} else {
						UInt32[] colors = new UInt32[16];
						GameboyConfig cfg = ConfigManager.Config.Gameboy;

						for(int i = 0; i < 4; i++) {
							colors[i] = cfg.BgColors[(ppu.BgPalette >> (i*2)) & 0x03];
							colors[i+4] = cfg.Obj0Colors[(ppu.ObjPalette0 >> (i * 2)) & 0x03];
							colors[i+8] = cfg.Obj1Colors[(ppu.ObjPalette1 >> (i * 2)) & 0x03];
						}

						colors[12] = 0xFFFFFFFF;
						colors[13] = 0xFFB0B0B0;
						colors[14] = 0xFF606060;
						colors[15] = 0xFF000000;
						PaletteColors = colors;
					}

					break;
				}
			}
		}
	}
}
