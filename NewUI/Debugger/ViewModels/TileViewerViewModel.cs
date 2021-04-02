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

		public void UpdatePaletteColors(byte[] cgram)
		{
			UInt32[] colors = new UInt32[256];
			for(int i = 0; i < 256; i++) {
				colors[i] = ToArgb(cgram[i * 2] | cgram[i * 2 + 1] << 8);
			}
			PaletteColors = colors;
		}
	}
}
