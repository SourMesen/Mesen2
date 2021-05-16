using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Avalonia;
using Mesen.Interop;

namespace Mesen.GUI.Config
{
	public class TileViewerConfig
	{
		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;

		public int ImageScale = 3;
		public bool ShowTileGrid = false;

		public SnesMemoryType Source = SnesMemoryType.VideoRam;
		public TileFormat Format = TileFormat.Bpp4;
		public TileLayout Layout = TileLayout.Normal;
		public TileBackground Background = TileBackground.Default;
		public int ColumnCount = 16;
		public int Address = 0;
		public int PageSize = 0x10000;
		public int SelectedPalette = 0;

		public bool AutoRefresh = true;
		public RefreshSpeed AutoRefreshSpeed = RefreshSpeed.Low;
		public int RefreshScanline = 240;
		public int RefreshCycle = 0;

		public TileViewerConfig()
		{
		}
	}
}
