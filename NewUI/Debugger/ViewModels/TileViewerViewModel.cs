using Avalonia.Controls;
using Mesen.Debugger.Controls;
using Mesen.GUI;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels
{
	public class TileViewerViewModel : ViewModelBase
	{
		[Reactive] public SnesMemoryType MemoryType { get; set; }
		[Reactive] public TileFormat TileFormat { get; set; }
		[Reactive] public TileLayout TileLayout { get; set; }
		[Reactive] public TileBackground TileBackground { get; set; }
		[Reactive] public bool ShowGrid { get; set; }
		[Reactive] public int SelectedPalette { get; set; }
		[Reactive] public int RowCount { get; set; }
		[Reactive] public int ColumnCount { get; set; }
		[Reactive] public int StartAddress { get; set; }
		[Reactive] public UInt32[] PaletteColors { get; set; }

		[ObservableAsProperty] public PaletteSelectionMode PaletteSelectionMode { get; set; }
		[ObservableAsProperty] public int AddressIncrement { get; set; }
		[ObservableAsProperty] public int MaximumAddress { get; set; }

		public TileViewerViewModel()
		{
			MemoryType = SnesMemoryType.VideoRam;
			TileFormat = TileFormat.Bpp2;
			TileLayout = TileLayout.Normal;
			TileBackground = TileBackground.Default;
			ShowGrid = false;
			SelectedPalette = 0;
			StartAddress = 0;
			ColumnCount = 32;
			RowCount = 64;

			this.WhenAnyValue(x => x.TileFormat).Select(x => {
				if(x == TileFormat.Bpp2) {
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
					default: bpp = 8; break;
				}
				return o.column * o.row * 8 * 8 * bpp / 8;
			}).ToPropertyEx(this, x => x.AddressIncrement);

			this.WhenAnyValue(x => x.MemoryType).Select(memType => {
				return DebugApi.GetMemorySize(memType);
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

		private UInt32[] _nesPalette = new UInt32[] { 0xFF666666, 0xFF002A88, 0xFF1412A7, 0xFF3B00A4, 0xFF5C007E, 0xFF6E0040, 0xFF6C0600, 0xFF561D00, 0xFF333500, 0xFF0B4800, 0xFF005200, 0xFF004F08, 0xFF00404D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFADADAD, 0xFF155FD9, 0xFF4240FF, 0xFF7527FE, 0xFFA01ACC, 0xFFB71E7B, 0xFFB53120, 0xFF994E00, 0xFF6B6D00, 0xFF388700, 0xFF0C9300, 0xFF008F32, 0xFF007C8D, 0xFF000000, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFF64B0FF, 0xFF9290FF, 0xFFC676FF, 0xFFF36AFF, 0xFFFE6ECC, 0xFFFE8170, 0xFFEA9E22, 0xFFBCBE00, 0xFF88D800, 0xFF5CE430, 0xFF45E082, 0xFF48CDDE, 0xFF4F4F4F, 0xFF000000, 0xFF000000, 0xFFFFFEFF, 0xFFC0DFFF, 0xFFD3D2FF, 0xFFE8C8FF, 0xFFFBC2FF, 0xFFFEC4EA, 0xFFFECCC5, 0xFFF7D8A5, 0xFFE4E594, 0xFFCFEF96, 0xFFBDF4AB, 0xFFB3F3CC, 0xFFB5EBF2, 0xFFB8B8B8, 0xFF000000, 0xFF000000 };

		public void UpdatePaletteColors()
		{
			bool isSnes = DebugApi.GetMemorySize(SnesMemoryType.CGRam) > 0;

			byte[] cgram = DebugApi.GetMemoryState(isSnes ? SnesMemoryType.CGRam : SnesMemoryType.NesPaletteRam);

			UInt32[] colors = new UInt32[256];
			if(isSnes) {
				for(int i = 0; i < 256; i++) {
					colors[i] = ToArgb(cgram[i * 2] | cgram[i * 2 + 1] << 8);
				}
			} else {
				for(int i = 0; i < 32; i++) {
					colors[i] = _nesPalette[cgram[i]];
				}
			}

			PaletteColors = colors;
		}
	}
}
